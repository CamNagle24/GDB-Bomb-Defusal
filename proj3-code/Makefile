SHELL = /bin/bash

zip: clean
	rm -f proj3-code.zip
	@if (( $$(find . -name "input.txt" | wc -l) < 1 )); then echo "ERROR: No input.txt file found. You must include this in your submission"; exit 1; fi
	@if (( $$(find . -name "bomb*" -type d | wc -l) < 1 )); then echo "ERROR: No bomb directory found. You must include this in your submission"; exit 1; fi
	zip -r proj3-code.zip *

clean:
	$(MAKE) -C bitwise clean

.PHONY: zip clean
