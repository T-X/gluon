/*
  Copyright (c) 2021, Linus Lüssing <linus.luessing@c0d3.blue>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <respondd.h>

#include <json-c/json.h>
#include <libgluonutil.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#define BPFCOUNTD_SOCKS_DIR "/var/run/bpfcountd/gluon-sockets"

enum capdir {
	CAP_IN,
	CAP_OUT,
	__CAP_COUNT,
};

static const char *dirstr[__CAP_COUNT] = {
	[CAP_IN] = "in",
	[CAP_OUT] = "out",
};

static int check_socket_suffix(char *socketname)
{
	unsigned int offset;

	if (strlen(socketname) <= strlen(".sock"))
		return -EINVAL;

	offset = strlen(socketname) - strlen(".sock");
	if (strcmp(&socketname[offset], ".sock"))
		return -EINVAL;

	return 0;
}

static int parse_socket_dir(char *socketname, enum capdir *dir)
{
	char *dir_start, *dir_end;

	dir_end = strrchr(socketname, '.');
	if (!dir_end)
		return -EINVAL;

	/* check for .in */
	dir_start = dir_end - strlen(".in");
	if (dir_start < socketname)
		return -EINVAL;

	if (!strncmp(dir_start, ".in", strlen(".in"))) {
		*dir = CAP_IN;
		return 0;
	}

	/* check for .out */
	dir_start = dir_end - strlen(".out");
	if (dir_start < socketname)
		return -EINVAL;

	if (!strncmp(dir_start, ".out", strlen(".out"))) {
		*dir = CAP_OUT;
		return 0;
	}

	return -EINVAL;
}

static int parse_socket_iface(char *socketname, enum capdir dir, char *iface)
{
	char *iface_start, *iface_end;

	/* skip UCI section name */
	iface_start = strchr(socketname, '.');
	if (!iface_start || iface_start[1] == '\0')
		return -EINVAL;

	iface_start += 1;
	iface_end = socketname + strlen(socketname);

	if (dir == CAP_IN)
		iface_end -= strlen(".in.sock");
	else if (dir == CAP_OUT)
		iface_end -= strlen(".out.sock");
	else
		return -EINVAL;

	if (*iface_end != '.' || iface_start >= iface_end ||
	    (iface_end - iface_start) >= IFNAMSIZ)
		return -EINVAL;

	memset(iface, 0, IFNAMSIZ);
	strncpy(iface, iface_start, iface_end - iface_start);

	return 0;
}

static struct json_object *get_json_new(struct json_object *parent,
					const char *key)
{
	struct json_object *child = NULL;
	json_bool ret;

	ret = json_object_object_get_ex(parent, key, &child);
	if (ret && child) {
		/* sanity check */
		if (!json_object_is_type(child, json_type_object))
			return NULL;

		return child;
	}

	child = json_object_new_object();
	if (!child)
		return NULL;

	/* json-c 0.12, should check return codes for >= 0.13 */
	json_object_object_add(parent, key, child);

	return child;
}

static struct json_object *parse_socket_name(char *socketname,
					     struct json_object *obj,
					     enum capdir *dir)
{
	int ret = check_socket_suffix(socketname);
	char iface[IFNAMSIZ];

	if (ret < 0)
		return NULL;

	ret = parse_socket_dir(socketname, dir);
	if (ret < 0)
		return NULL;

	ret = parse_socket_iface(socketname, *dir, iface);
	if (ret < 0)
		return NULL;

	return get_json_new(obj, iface);
}

static int parse_num(const char *start, int64_t *num)
{
	unsigned long long __num;
	char *endptr;

	if (*start == '-')
		return -EINVAL;

	__num = strtoull(start, &endptr, 10);
	if ((__num == ULLONG_MAX && errno == ERANGE) ||
	    start == endptr)
		return -EINVAL;

	/* limit to int64 max for json-c */
	if (__num > INT64_MAX)
		return -EINVAL;

	*num = (int64_t)__num;
	return 0;
}

static int add_num_to_json(struct json_object *dir_obj, const char *key,
			  int64_t *num)
{
	struct json_object *elem;

	elem = json_object_new_int64(*num);
	if (!elem)
		return -ENOMEM;

	/* json-c < 0.13 */
	json_object_object_add(dir_obj, key, elem);
	return 0;
}

/*
 * Example:
 * {
 *   "bytes": 12345,
 *   "packets": 456
 * }
 */
static struct json_object *create_dir_object(int64_t *bytes,  int64_t *packets)
{
	struct json_object *dir_obj;
	int ret;

	dir_obj = json_object_new_object();
	if (!dir_obj)
		return NULL;

	ret = add_num_to_json(dir_obj, "bytes", bytes);
	if (ret < 0)
		goto err;

	ret = add_num_to_json(dir_obj, "packets", packets);
	if (ret < 0)
		goto err;

	return dir_obj;
err:
	json_object_put(dir_obj);
	return NULL;
}

static int add_line_to_json(struct json_object *iface_obj, enum capdir dir,
			    const char *rule, int64_t *bytes, int64_t *packets)
{
	struct json_object *dir_obj, *rule_obj = NULL;
	int ret;

	dir_obj = create_dir_object(bytes, packets);
	if (!dir_obj)
		return -ENOMEM;

	rule_obj = get_json_new(iface_obj, rule);
	if (!rule_obj)
		goto err;

	/* sanity check, should not exist yet */
	ret = json_object_object_get_ex(rule_obj, dirstr[dir], NULL);
	if (ret)
		goto err;

	/* json-c < 0.13 */
	json_object_object_add(rule_obj, dirstr[dir], dir_obj);
	return 0;
err:
	json_object_put(dir_obj);
	return -ENOMEM;
}

static int parse_line(struct json_object *obj, enum capdir dir, char *line)
{
	int64_t bytes;
	int64_t packets;
	char *start;
	int ret;

	/* skip lines prefixed with '%' */
	if (line[0] == '%')
		return 0;

	/* packets */
	start = strrchr(line, ':');
	if (!start)
		return -EINVAL;

	ret = parse_num(&start[1], &packets);
	if (ret < 0)
		return ret;

	*start = '\0';

	/* bytes */
	start = strrchr(line, ':');
	if (!start)
		return -EINVAL;

	ret = parse_num(&start[1], &bytes);
	if (ret < 0)
		return ret;

	*start = '\0';
	/* line: is now rule name */

	ret = add_line_to_json(obj, dir, line, &bytes, &packets);
	if (ret < 0)
		return -EINVAL;

	return 0;
}

static int parse_socket(const char *socketname, struct json_object *iface_obj,
			enum capdir dir)
{
	char linebuf[1024];
	struct sockaddr_un addr;
	int sd, ret;
	FILE *file;

	sd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sd < 0)
		return sd;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, socketname, sizeof(addr.sun_path) - 1);

	ret = connect(sd, (const struct sockaddr *)&addr,
		      sizeof(addr));
	if (ret < 0)
		goto out;

	file = fdopen(sd, "r");
	if (!file) {
		ret = -EACCES;
		goto out;
	}

	/* Does this really help or maybe even worsen performance? */
	setlinebuf(file);

	while (!feof(file)) {
		if (!fgets(linebuf, sizeof(linebuf)-1, file))
			break;

		parse_line(iface_obj, dir, linebuf);
	}

	ret = 0;
out:
	close(sd);
	return ret;
}

static void parse_socket_subdir(struct dirent *subdir, struct json_object *obj)
{
	struct json_object *subdir_obj;
	struct dirent *dp;
	DIR *dir;

	if (chdir(subdir->d_name) < 0)
		return;

	dir = opendir("./");
	if (!dir)
		goto err;

	subdir_obj = json_object_new_object();
	if (!subdir_obj)
		goto err2;

	json_object_object_add(obj, subdir->d_name, subdir_obj);

	while ((dp = readdir(dir))) {
		struct json_object *iface_obj;
		enum capdir capdir;

		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;

		iface_obj = parse_socket_name(dp->d_name, subdir_obj, &capdir);
		if (!iface_obj)
			continue;

		parse_socket(dp->d_name, iface_obj, capdir);
	}

err2:
	closedir(dir);
err:
	chdir("..");
}

static struct json_object *get_mcaststats(void)
{
	struct json_object *obj;
	struct dirent *dp;
	DIR *dir;

	obj = json_object_new_object();
	if (!obj)
		return NULL;

	if (chdir(BPFCOUNTD_SOCKS_DIR) < 0)
		return obj;

	dir = opendir("./");
	if (!dir)
		return obj;

	while ((dp = readdir(dir))) {
		if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
			continue;

		parse_socket_subdir(dp, obj);
	}

	closedir(dir);
	return obj;
}

static struct json_object *respondd_provider_mcaststats(void)
{
	struct json_object *ret, *traffic, *mcaststats;

	ret = json_object_new_object();
	if (!ret)
		return NULL;

	traffic = json_object_new_object();
	if (!traffic)
		goto err1;

	mcaststats = get_mcaststats();
	if (!mcaststats)
		goto err2;

	/* json-c 0.12, should check return codes for >= 0.13 */
	json_object_object_add(traffic, "mcast", mcaststats);
	json_object_object_add(ret, "traffic", traffic);

	return ret;

err2:
	json_object_put(traffic);
err1:
	json_object_put(ret);
	return NULL;
}

/*
 * Example output:
 *
 * {
 *   "mesh-vpn": {
 *     "mesh": {
 *       "BAT-BCAST-ARP": {
 *         "in": {
 *           "bytes": 12345,
 *           "packets": 456
 *         },
 *         "out": {
 *           "bytes": 112233,
 *           "packets": 11
 *         }
 *       },
 *       "BAT-BCAST-IP6": {
 *         "in": {
 *           "bytes": 12345,
 *           "packets": 456
 *         "out": {
 *           "bytes": 112233,
 *           "packets": 11
 *         }
 *       }
 *     }
 *   }
 * }
*/
const struct respondd_provider_info respondd_providers[] = {
	{"statistics-extended", respondd_provider_mcaststats},
	{}
};
