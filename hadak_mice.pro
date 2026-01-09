TEMPLATE = subdirs

SUBDIRS += app tests

app.file = src/hadak_mice.pro
tests.file = tests/tests.pro
tests.depends = app
