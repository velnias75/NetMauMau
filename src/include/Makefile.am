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
	-@rm $(top_srcdir)/src/server/ai-icon.h
	-@rm $(top_builddir)/src/server/ai-icon.h
	-touch ai-icon-deleted
endif
