
CONFIG_FILES = gravis.cfg gsdrum.cfg midia.cfg mt32.cfg sndfont.cfg timidity.cfg wowpats.cfg \
	chaos12-voices chaos8-voices megadrum megainst pila-voices sound-canvas-drums
TIMID_DIR = $(kde_datadir)/kmidi/config

install-data-local:
	$(mkinstalldirs) $(TIMID_DIR)
	@for i in $(CONFIG_FILES); do \
	  if [ ! -e $(TIMID_DIR)/$$i ]; then  \
	    echo "$(INSTALL_DATA) $$i $(TIMID_DIR)"; \
	    $(INSTALL_DATA) $$i $(TIMID_DIR) ; \
          fi ; \
        done

uninstall-local:
	for i in $(CONFIG_FILES); do \
	  rm -f $(TIMID_DIR)/$$i ; \
	done

