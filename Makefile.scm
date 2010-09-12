all:
	autoreconf --install --force
	
clean:
	hg status -in | xargs -r rm -rf
	find -type d -empty -print | xargs -r rmdir
