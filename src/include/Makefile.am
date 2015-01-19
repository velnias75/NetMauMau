pkginclude_HEADERS = abstractclient.h abstractconnection.h abstractsocket.h \
	capabilitiesexception.h cardtools.h clientconnection.h connectionrejectedexception.h \
	ibase64.h icard.h iplayerpiclistener.h linkercontrol.h nonetmaumauserverexception.h \
	playerlistexception.h protocolerrorexception.h scoresexception.h shutdownexception.h \
	socketexception.h timeoutexception.h versionmismatchexception.h 

if BRANDING
	
noinst_DATA = ai-icon-deleted
	
clean-local:
	-rm -f ai-icon-deleted

ai-icon-deleted: $(AI_IMAGE)
	-@rm -f $(top_srcdir)/src/server/ai-icon.h 2> /dev/null
	-@rm -f $(top_builddir)/src/server/ai-icon.h 2> /dev/null
	-touch ai-icon-deleted
endif
