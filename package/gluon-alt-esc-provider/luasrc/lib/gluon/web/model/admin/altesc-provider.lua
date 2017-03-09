local uci = require("simple-uci").cursor()
local util = require 'gluon.util'
local site = require 'gluon.site_config'

local function get_provider(uci)
  local provider
  uci:foreach('gluon-alt-esc-provider', 'provider',
              function(s)
                 provider = s
                 return false
              end
  )
  return provider
end

local mac = uci:get('network', 'client', 'macaddr')
local disabled = uci:get_first('gluon-alt-esc-provider', 'provider', "disabled")

local f = Form(translate("Alternative Exit Service Collaborator - Provider"))
local s = f:section(Section, nil, translate(
		'<p>This allows you to share an internet connection from the WAN port directly.</p>'
                .. '<p>Be aware of the legal obligations your jurisdiction might require you to follow.</p>'
                .. '<p>USE AT YOUR OWN RISK!</p>'
))

local enabled = s:option(Flag, "enabled", translate("Enabled"))
enabled.default = disabled and disabled == "0"

local brave = s:option(Flag, "brave", translate("I am brave and know what I am doing."))
brave:depends(enabled, true)
brave.default = disabled and disabled == "0"

local id = s:option(Value, "id", translate("Your Exit ID is:"), translate("(unchangeable)"))
id:depends(brave, true)
id.default = mac

function f:write(self, state, data)
  local disabled
  local provider = get_provider(uci)['.name']

  if not(enabled.data and brave.data) then
    disabled = "1"

    uci:delete('firewall', 'client2wan')
    uci:delete('firewall', 'wan2client')
  else
    disabled = "0"

    uci:section('firewall', 'forwarding', 'client2wan',
                {
                  src = 'client',
                  dest = 'wan',
                }
    )
    uci:section('firewall', 'forwarding', 'wan2client',
                {
                  src = 'wan',
                  dest = 'client',
                }
    )
  end

  uci:set('gluon-alt-esc-provider', provider, 'disabled', disabled)
  uci:commit('gluon-alt-esc-provider')
  uci:commit('firewall')
end

return f
