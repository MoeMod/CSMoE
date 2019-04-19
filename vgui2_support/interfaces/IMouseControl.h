#ifndef IMOUSECONTROL_H
#define IMOUSECONTROL_H

class IMouseControl {
public:
	virtual bool VGUI2MouseControl() = 0;
	virtual void SetVGUI2MouseControl(bool) = 0;
};

#endif // IMOUSECONTROL_H
