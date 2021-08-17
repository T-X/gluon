include $(TOPDIR)/rules.mk

PKG_NAME:=bpfcountd
PKG_SOURCE_DATE:=2020-01-01
PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=https://github.com/lemoer/bpfcountd.git
PKG_SOURCE_VERSION:=9f602a1e8e691749ca233ede06846c7e53623bc5

PKG_BUILD_DEPENDS += libpcap

include $(INCLUDE_DIR)/package.mk

define Package/bpfcountd
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Berkeley Packet Filter Counting Daemon
  DEPENDS:= +libpcap
endef

define Package/bpfcountd/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bpfcountd $(1)/usr/sbin/
	$(CP) ./files/* $(1)/
endef

$(eval $(call BuildPackage,bpfcountd))
