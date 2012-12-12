

sysconf_DATA = uplogd.conf


uplogd.conf: FLAG
	@echo "Create uplogd.conf >>>>>"
	@echo "# This is an example configuration file for uplogd."    >  $@
	@echo "# It contains the default settings that would be used " >> $@
	@echo "# if there was no uplogd.conf file."                    >> $@
	@echo "# "                                                     >> $@
	@echo ""                                                       >> $@
	@echo "# Message log file"                                     >> $@
	@echo "#logfile = '"$(UPLOGGER_LOGDIR)"/uplog.log'"            >> $@
	@echo ""                                                       >> $@
	@echo "# Socket filename"                                      >> $@
	@echo "#sockfile ='"$(UPLOGGER_SOCKDIR)"/uplogd.pid'"          >> $@

