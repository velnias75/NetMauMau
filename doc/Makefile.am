EXTRA_DIST = DoxygenLayout.xml

if GENDOX

noinst_DATA = client-dox.stamp
	
clean-local:
	-rm -rf html
	-rm -rf client.dox.warnings client-dox.stamp

client-dox.stamp: client.dox
	$(DOXYGEN) $<
	touch client-dox.stamp
	
install-data-local:
	$(INSTALL) -d -m 755 $(DESTDIR)/$(docdir)/client-api-doc
	$(INSTALL) -m 644 -D html/* $(DESTDIR)/$(docdir)/client-api-doc
	-rm -rf $(DESTDIR)/$(docdir)/client-api-doc/*.map
	-rm -rf $(DESTDIR)/$(docdir)/client-api-doc/*.md5
	
uninstall-local:
	-rm -rf $(DESTDIR)/$(docdir)/client-api-doc

endif
