include $(TOPDIR)/rules.mk

PKG_NAME:=gluon-statistics-mcast
PKG_VERSION:=1

include ../gluon.mk

define Package/gluon-statistics-mcast
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Gluon Statistics for Multicast Traffic
  DEPENDS:= +bpfcountd +libgluonutil +luasocket
endef

$(eval $(call BuildPackageGluon,gluon-statistics-mcast))
