INSTALLDIR=/usr/local
DISTDIR=stepp-tagger

CFLAGS = -O2 -DNDEBUG
#CFLAGS = -pg -O5
#CFLAGS = -g -Wall -W
CPP = g++
#CPP = g++
LOBJS = learn.cpp maxent.o eftag.o crf.o lbfgs.o crfpos.o owlqn.o
MOBJS = main.cpp maxent.o tokenize.o eftag.o crf.o crfpos.o lbfgs.o owlqn.o

all: stepp-eval stepp-learn stepp

install: stepp
	mkdir -p $(INSTALLDIR)/bin && cp stepp $(INSTALLDIR)/bin/
	mkdir -p $(INSTALLDIR)/share/stepp/models_wsj02-21c && cp models_wsj02-21c/model.* models_wsj02-21c/dictionary.txt $(INSTALLDIR)/share/stepp/models_wsj02-21c/
	mkdir -p $(INSTALLDIR)/share/stepp/models_medline && cp models_medline/model.* $(INSTALLDIR)/share/stepp/models_medline/
	mkdir -p $(INSTALLDIR)/share/stepp/models_brown-wsj02-21c && cp models_brown-wsj02-21c/model.* models_brown-wsj02-21c/dictionary.txt $(INSTALLDIR)/share/stepp/models_brown-wsj02-21c/
	mkdir -p $(INSTALLDIR)/share/stepp/models_wsj02-21-IQc && cp models_wsj02-21-IQc/model.* models_wsj02-21-IQc/dictionary.txt $(INSTALLDIR)/share/stepp/models_wsj02-21-IQc/

dist: stepp
	rm -rf $(DISTDIR)
	mkdir -p $(DISTDIR) && cp README Makefile *.h *.cpp $(DISTDIR)
	mkdir -p $(DISTDIR)/models_wsj02-21c && cp models_wsj02-21c/model.* models_wsj02-21c/dictionary.txt $(DISTDIR)/models_wsj02-21c
	mkdir -p $(DISTDIR)/models_medline && cp models_medline/model.* $(DISTDIR)/models_medline
	mkdir -p $(DISTDIR)/models_brown-wsj02-21c && cp models_brown-wsj02-21c/model.* models_brown-wsj02-21c/dictionary.txt $(DISTDIR)/models_brown-wsj02-21c
	mkdir -p $(DISTDIR)/models_wsj02-21-IQc && cp models_wsj02-21-IQc/model.* models_wsj02-21-IQc/dictionary.txt $(DISTDIR)/models_wsj02-21-IQc

stepp-learn: $(LOBJS)
	$(CPP) -o stepp-learn $(CFLAGS) $(LOBJS)
stepp: $(MOBJS)
	$(CPP) -o stepp $(CFLAGS) $(MOBJS)
stepp-eval: eval.cpp
	$(CPP) -o stepp-eval $(CFLAGS) eval.cpp
clean:
	/bin/rm -r -f $(OBJS) a.out stepp stepp-learn stepp-eval *.o *~
.cpp.o:
	$(CPP) -c $(CFLAGS) $<
