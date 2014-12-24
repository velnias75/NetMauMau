pkginclude_HEADERS = abstractclient.h clientconnection.h timeoutexception.h icard.h \
	linkercontrol.h abstractconnection.h abstractsocket.h socketexception.h cardtools.h \
	shutdownexception.h playerlistexception.h capabilitiesexception.h protocolerrorexception.h \
	connectionrejectedexception.h versionmismatchexception.h nonetmaumauserverexception.h \
	iplayerpiclistener.h

if BRANDING
	
noinst_DATA = ai-icon-deleted
	
clean-local:
	-rm -f ai-icon-deleted

ai-icon-deleted: $(AI_IMAGE)
	-@rm -f $(top_srcdir)/src/server/ai-icon.h 2> /dev/null
	-@rm -f $(top_builddir)/src/server/ai-icon.h 2> /dev/null
	-touch ai-icon-deleted
endif
