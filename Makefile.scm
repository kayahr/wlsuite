all:
	mkdir -p m4
	autoreconf --install --force -I m4
	
clean:
	rm -rf $$(cat .gitignore)
