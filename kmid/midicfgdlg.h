#include <qdialog.h>

class DeviceManager;

class QLabel;
class QPushButton;
class QListBox;
class MidiConfigDialog : public QDialog
{
	Q_OBJECT

public:

	MidiConfigDialog(DeviceManager *dm,QWidget *parent,const char *name);

public slots:
	void deviceselected(int idx);
	void browseMap();
	void noMap();

private:
QLabel *label;
QLabel *label2;
QLabel *maplabel;
QPushButton *ok;
QPushButton *cancel;
QListBox *mididevices;
QPushButton *mapbrowse;
QPushButton *mapnone;

DeviceManager *devman;

public:
static int selecteddevice;
static char *selectedmap;
};
