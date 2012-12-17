SUBDIRS =       \
	src         \
	conf        \
	init-redhat


install-data-local:
	test -z "$(UPLOGGER_LOGDIR)" || $(MKDIR_P) "$(DESTDIR)$(UPLOGGER_LOGDIR)"
	test -z "$(UPLOGGER_PIDDIR)" || $(MKDIR_P) "$(DESTDIR)$(UPLOGGER_PIDDIR)"


install-init-redhat:
	@$(MAKE) $(AM_MAKEFLAGS) -C ./init-redhat install-init-redhat

