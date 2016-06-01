# Global parameters
GCC = g++.exe
FLAGS =	-Wall -fmessage-length=0 -I..\utilities\inc -O2 -DDEBUG
TARGET = bin\lambda.exe

all: $(TARGET)

# Create target application
$(TARGET): obj\main.o obj\menu.o obj\tools.o obj\editor.o obj\explorer.o obj\scrolledit.o obj\splitter.o obj\tabs.o obj\toolbar.o obj\aboutdialog.o obj\datetimedialog.o obj\finddialog.o obj\gotodialog.o obj\inputdialog.o obj\preferencesdialog.o obj\propertiesdialog.o obj\document.o obj\documentlist.o obj\observable.o obj\stringlist.o res\resource.res
	$(GCC) -mwindows -o $@ $^ -l comctl32

#Resource
res\resource.res: res\resource.rc
	windres.exe --input-format=rc -O coff -o $@ -i $<

#Root
obj\main.o: src\main.cpp
	$(GCC) $(FLAGS) -o $@ -c $<

obj\menu.o: src\menu.cpp src\menu.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\tools.o: src\tools.cpp src\tools.h
	$(GCC) $(FLAGS) -o $@ -c $<

#Controls
obj\editor.o: src\controls\editor.cpp src\controls\editor.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\explorer.o: src\controls\explorer.cpp src\controls\explorer.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\scrolledit.o: src\controls\scrolledit.cpp src\controls\scrolledit.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\splitter.o: src\controls\splitter.cpp src\controls\splitter.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\tabs.o: src\controls\tabs.cpp src\controls\tabs.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\toolbar.o: src\controls\toolbar.cpp src\controls\toolbar.h
	$(GCC) $(FLAGS) -o $@ -c $<

#Dialogs
obj\aboutdialog.o: src\dialogs\aboutdialog.cpp src\dialogs\aboutdialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\datetimedialog.o: src\dialogs\datetimedialog.cpp src\dialogs\datetimedialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\finddialog.o: src\dialogs\finddialog.cpp src\dialogs\finddialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\gotodialog.o: src\dialogs\gotodialog.cpp src\dialogs\gotodialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\inputdialog.o: src\dialogs\inputdialog.cpp src\dialogs\inputdialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\preferencesdialog.o: src\dialogs\preferencesdialog.cpp src\dialogs\preferencesdialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\propertiesdialog.o: src\dialogs\propertiesdialog.cpp src\dialogs\propertiesdialog.h
	$(GCC) $(FLAGS) -o $@ -c $<

#Models
obj\document.o: src\models\document.cpp src\models\document.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\documentlist.o: src\models\documentlist.cpp src\models\documentlist.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\observable.o: src\models\observable.cpp src\models\observable.h
	$(GCC) $(FLAGS) -o $@ -c $<

obj\stringlist.o: src\models\stringlist.cpp src\models\stringlist.h
	$(GCC) $(FLAGS) -o $@ -c $<

# Clean targets
clean:
	del obj\*.o
	del res\*.res
	del $(TARGET)
