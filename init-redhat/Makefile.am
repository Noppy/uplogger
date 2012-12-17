
initddir  = /etc/rc.d/init.d

CHKCONFIG = /sbin/chkconfig

EXTRA_DIST = uplogd_template


install-init-redhat: uplogd
	@cp $< "$(initddir)/$<"
	@chmod 755  "$(initddir)/$<"
	@chown root:root "$(initddir)/$<"
	@$(CHKCONFIG) --add "$<"
	@$(CHKCONFIG) "$<" on


uplogd: FLAG uplogd_template
	@sed "s|_MAKE_SBINPATH_|${sbindir}|" uplogd_template > $@

