!function(){var e={};function t(e){return/^-?\d+$/.test(e)?+e:NaN}function n(e){return/^-?\d*\.?\d+?$/.test(e)?+e:NaN}var a={integer:function(){return!isNaN(t(this))},uinteger:function(){return t(this)>=0},float:function(){return!isNaN(n(this))},ufloat:function(){return n(this)>=0},ipaddr:function(){return a.ip4addr.apply(this)||a.ip6addr.apply(this)},ip4addr:function(){var e;return!!(e=this.match(/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/))&&(e[1]>=0&&e[1]<=255&&e[2]>=0&&e[2]<=255&&e[3]>=0&&e[3]<=255&&e[4]>=0&&e[4]<=255)},ip6addr:function(){return this.indexOf("::")<0?null!=this.match(/^(?:[a-f0-9]{1,4}:){7}[a-f0-9]{1,4}$/i):!(this.indexOf(":::")>=0||this.match(/::.+::/)||this.match(/^:[^:]/)||this.match(/[^:]:$/))&&(!!this.match(/^(?:[a-f0-9]{0,4}:){2,7}[a-f0-9]{0,4}$/i)||(!!this.match(/^(?:[a-f0-9]{1,4}:){7}:$/i)||!!this.match(/^:(?::[a-f0-9]{1,4}){7}$/i)))},wpakey:function(){var e=this;return 64==e.length?null!=e.match(/^[a-f0-9]{64}$/i):e.length>=8&&e.length<=63},range:function(e,t){var a=n(this);return a>=+e&&a<=+t},min:function(e){return n(this)>=+e},max:function(e){return n(this)<=+e},irange:function(e,n){var a=t(this);return a>=+e&&a<=+n},imin:function(e){return t(this)>=+e},imax:function(e){return t(this)<=+e},minlength:function(e){return(""+this).length>=+e},maxlength:function(e){return(""+this).length<=+e}};function r(e){for(var t=0;t<e.length;t++){var n=!0;for(var a in e[t])n=n&&(r=a,i=e[t][a],o=void 0,(o=document.getElementById(r))?("checkbox"==o.type?o.checked:o.value?o.value:"")==i:"radio"==(o=document.getElementById(r+"."+i)).type&&o.checked);if(n)return!0}var r,i,o;return!1}function i(){var t=!1;for(var n in e){var a=e[n],o=document.getElementById(n),d=document.getElementById(a.parent);if(o&&o.parentNode&&!r(a.deps))o.parentNode.removeChild(o),t=!0;else if(d&&(!o||!o.parentNode)&&r(a.deps)){var u=void 0;for(u=d.firstChild;u&&!(u.getAttribute&&parseInt(u.getAttribute("data-index"),10)>a.index);u=u.nextSibling);u?d.insertBefore(a.node,u):d.appendChild(a.node),t=!0}d&&d.parentNode&&d.getAttribute("data-optionals")&&(d.parentNode.style.display=d.options.length<=1?"none":"")}t&&i()}function o(e,t,n,a){return e.addEventListener?e.addEventListener(t,n,!!a):e.attachEvent("on"+t,function(){var e=window.event;return!e.target&&e.srcElement&&(e.target=e.srcElement),!!n(e)}),e}function d(e,t){var n=t.prefix;function a(a,l,s){for(var c=[];e.firstChild;){var p=e.firstChild;(f=+p.index)!=s&&("input"==p.nodeName.toLowerCase()?c.push(p.value||""):"select"==p.nodeName.toLowerCase()&&(c[c.length-1]=p.options[p.selectedIndex].value)),e.removeChild(p)}l>=0?(a=l+1,c.splice(l,0,"")):t.optional||0!=c.length||c.push("");for(var f=1;f<=c.length;f++){var v=document.createElement("input");if(v.id=n+"."+f,v.name=n,v.value=c[f-1],v.type="text",v.index=f,v.className="gluon-input-text",t.size&&(v.size=t.size),t.placeholder&&(v.placeholder=t.placeholder),e.appendChild(v),t.type&&u(v,!1,t.type),o(v,"keydown",i),o(v,"keypress",r),f==a)v.focus();else if(-f==a){v.focus();var h=v.value;v.value=" ",v.value=h}if(t.optional||c.length>1)(m=document.createElement("span")).className="gluon-remove",e.appendChild(m),o(m,"click",d(!1)),e.appendChild(document.createElement("br"))}var m;(m=document.createElement("span")).className="gluon-add",e.appendChild(m),o(m,"click",d(!0))}function r(e){var t=(e=e||window.event).target?e.target:e.srcElement;switch(3==t.nodeType&&(t=t.parentNode),e.keyCode){case 8:case 46:return 0!=t.value.length||(e.preventDefault&&e.preventDefault(),!1);case 13:case 38:case 40:return e.preventDefault&&e.preventDefault(),!1}return!0}function i(e){var t,r,i=(e=e||window.event).target?e.target:e.srcElement,o=0;if(i){for(3==i.nodeType&&(i=i.parentNode),o=i.index,t=i.previousSibling;t&&t.name!=n;)t=t.previousSibling;for(r=i.nextSibling;r&&r.name!=n;)r=r.nextSibling}switch(e.keyCode){case 8:case 46:if("select"==i.nodeName.toLowerCase()||0==i.value.length){e.preventDefault&&e.preventDefault();var d=i.index;return 8==e.keyCode&&(d=1-d),a(d,-1,o),!1}break;case 13:a(-1,o,-1);break;case 38:t&&t.focus();break;case 40:r&&r.focus()}return!0}function d(e){return function(t){for(var a=((t=t||window.event).target?t.target:t.srcElement).previousSibling;a&&a.name!=n;)a=a.previousSibling;return e?i({target:a,keyCode:13}):(a.value="",i({target:a,keyCode:8})),!1}}a(NaN,-1,-1)}function u(e,t,n){var r,i,d,u=(d=(r=n).match(/^([^\(]+)\(([^,]+),([^\)]+)\)$/))&&void 0!==(i=a[d[1]])?function(){return i.apply(this,[d[2],d[3]])}:(d=r.match(/^([^\(]+)\(([^,\)]+)\)$/))&&void 0!==(i=a[d[1]])?function(){return i.apply(this,[d[2]])}:a[r];if(u){var l=function(){if(e.form){e.className=e.className.replace(/ gluon-input-invalid/g,"");var n=e.options&&e.options.selectedIndex>-1?e.options[e.options.selectedIndex].value:e.value;0==n.length&&t||u.apply(n)||(e.className+=" gluon-input-invalid")}};o(e,"blur",l),o(e,"keyup",l),"select"==e.nodeName.toLowerCase()&&(o(e,"change",l),o(e,"click",l)),l()}}!function(){var t,n,a,r,l;t=document.querySelectorAll("[data-depends]");for(var s=0;void 0!==(g=t[s]);s++){var c=parseInt(g.getAttribute("data-index"),10),p=JSON.parse(g.getAttribute("data-depends"));if(!isNaN(c)&&p.length>0)for(var f=0;f<p.length;f++)n=g,a=p[f],r=c,l=void 0,(l=e[n.id])||(l={node:n,parent:n.parentNode.id,deps:[],index:r},e[n.id]=l),l.deps.push(a)}t=document.querySelectorAll("[data-update]");for(s=0;void 0!==(g=t[s]);s++)for(var v,h=g.getAttribute("data-update").split(" "),m=0;void 0!==(v=h[m]);m++)o(g,v,i);t=document.querySelectorAll("[data-type]");for(s=0;void 0!==(g=t[s]);s++)u(g,"true"===g.getAttribute("data-optional"),g.getAttribute("data-type"));t=document.querySelectorAll("[data-dynlist]");var g;for(s=0;void 0!==(g=t[s]);s++){d(g,JSON.parse(g.getAttribute("data-dynlist")))}i()}()}();