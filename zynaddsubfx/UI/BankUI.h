// generated by Fast Light User Interface Designer (fluid) version 1.0300

#ifndef BankUI_h
#define BankUI_h
#include <FL/Fl.H>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <FL/Fl_Button.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Input.H>
#include "Fl_Osc_Interface.h"
#include "Fl_Osc_Check.H"
#include "Fl_Osc_Pane.H"
#include "../Misc/Util.h"
#include "BankView.h"
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>

class BankUI {
public:
  Fl_Osc_Window* make_window();
  Fl_Osc_Window *bankuiwindow;
private:
  void cb_Close_i(Fl_Button*, void*);
  static void cb_Close(Fl_Button*, void*);
public:
  BankView *bankview;
  BankViewControls *modeselect;
private:
  void cb_New_i(Fl_Button*, void*);
  static void cb_New(Fl_Button*, void*);
public:
  BankList *banklist;
private:
  void cb_banklist_i(BankList*, void*);
  static void cb_banklist(BankList*, void*);
  void cb_Refresh_i(Fl_Button*, void*);
  static void cb_Refresh(Fl_Button*, void*);
  void cb_Search_i(Fl_Input*, void*);
  static void cb_Search(Fl_Input*, void*);
public:
  BankUI(int *npart_, Fl_Osc_Interface *osc_);
  virtual ~BankUI();
  void show();
  void hide();
  void init(Fl_Valuator *cbwig_);
  void process();
  void refreshmainwindow();
  void rescan_for_banks();
  void update_search(std::string search_string);
private:
  Fl_Osc_Interface *osc; 
public:
  Fl_Valuator *cbwig; 
private:
  int *npart; 
};
#endif
