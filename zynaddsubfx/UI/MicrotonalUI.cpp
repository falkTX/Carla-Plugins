// generated by Fast Light User Interface Designer (fluid) version 1.0300

#include "external/zynaddsubfx/UI/MicrotonalUI.h"
//Copyright (c) 2002-2005 Nasca Octavian Paul
//License: GNU GPL version 2 or later

void MicrotonalUI::cb_Invert_i(Fl_Osc_Check* o, void*) {
  if (o->value()==0) centerinvertcounter->deactivate();
   else centerinvertcounter->activate();
}
void MicrotonalUI::cb_Invert(Fl_Osc_Check* o, void* v) {
  ((MicrotonalUI*)(o->parent()->parent()->user_data()))->cb_Invert_i(o,v);
}

void MicrotonalUI::cb_applybutton_i(Fl_Button*, void*) {
  apply();
}
void MicrotonalUI::cb_applybutton(Fl_Button* o, void* v) {
  ((MicrotonalUI*)(o->parent()->parent()->user_data()))->cb_applybutton_i(o,v);
}

void MicrotonalUI::cb_Import_i(Fl_Button*, void*) {
  const char *filename;
filename=fl_file_chooser("Open:","(*.scl)",NULL,0);
if (filename==NULL) return;
osc->write("/load_scl", "s", filename);
if (true) {
       updateTuningsInput();
       nameinput->update();
       commentinput->update();
       tuningsinput->position(0);
       octavesizeoutput->update();
     };
}
void MicrotonalUI::cb_Import(Fl_Button* o, void* v) {
  ((MicrotonalUI*)(o->parent()->parent()->user_data()))->cb_Import_i(o,v);
}

void MicrotonalUI::cb_mappingenabledbutton_i(Fl_Osc_Check* o, void*) {
  if (o->value()==0) keymappinggroup->deactivate();
 else keymappinggroup->activate();
o->show();
}
void MicrotonalUI::cb_mappingenabledbutton(Fl_Osc_Check* o, void* v) {
  ((MicrotonalUI*)(o->parent()->parent()->user_data()))->cb_mappingenabledbutton_i(o,v);
}

void MicrotonalUI::cb_Import1_i(Fl_Button*, void*) {
  const char *filename;
filename=fl_file_chooser("Open:","(*.kbm)",NULL,0);
if (filename==NULL) return;
osc->write("/load_kbm", "s", filename);
if (true) {
       updateMappingInput();
       mappinginput->position(0);
       mapsizeoutput->update();
       firstnotecounter->update();
       lastnotecounter->update();
       middlenotecounter->update();
       mappingenabledbutton->update();
       afreqinput->update();
       anotecounter->update();
     };
}
void MicrotonalUI::cb_Import1(Fl_Button* o, void* v) {
  ((MicrotonalUI*)(o->parent()->parent()->user_data()))->cb_Import1_i(o,v);
}

void MicrotonalUI::cb_anotecounter_i(Fl_Osc_Counter* o, void*) {
  /*if (microtonal->getnotefreq(o->value(),0)<0.0) o->textcolor(FL_RED);
 else o->textcolor(FL_BLACK);*/

o->redraw();
}
void MicrotonalUI::cb_anotecounter(Fl_Osc_Counter* o, void* v) {
  ((MicrotonalUI*)(o->parent()->parent()->user_data()))->cb_anotecounter_i(o,v);
}

void MicrotonalUI::cb_Close_i(Fl_Button*, void*) {
  microtonaluiwindow->hide();
}
void MicrotonalUI::cb_Close(Fl_Button* o, void* v) {
  ((MicrotonalUI*)(o->parent()->user_data()))->cb_Close_i(o,v);
}

void MicrotonalUI::cb_Enable_i(Fl_Osc_Check* o, void*) {
  if (o->value()==0) microtonalgroup->deactivate();
   else microtonalgroup->activate();
}
void MicrotonalUI::cb_Enable(Fl_Osc_Check* o, void* v) {
  ((MicrotonalUI*)(o->parent()->user_data()))->cb_Enable_i(o,v);
}

Fl_Osc_Window* MicrotonalUI::make_window(Fl_Osc_Interface *osc, std::string base) {
  { microtonaluiwindow = new Fl_Osc_Window(405, 450, "Scales");
    microtonaluiwindow->box(FL_FLAT_BOX);
    microtonaluiwindow->color(FL_BACKGROUND_COLOR);
    microtonaluiwindow->selection_color(FL_BACKGROUND_COLOR);
    microtonaluiwindow->labeltype(FL_NO_LABEL);
    microtonaluiwindow->labelfont(0);
    microtonaluiwindow->labelsize(14);
    microtonaluiwindow->labelcolor(FL_FOREGROUND_COLOR);
    microtonaluiwindow->user_data((void*)(this));
    microtonaluiwindow->align(Fl_Align(FL_ALIGN_TOP));
    microtonaluiwindow->when(FL_WHEN_RELEASE);
    { new Fl_Box(0, 0, 0, 0);
      microtonaluiwindow->init(osc, base);
    } // Fl_Box* o
    { Fl_Group* o = new Fl_Group(249, 2, 155, 45);
      o->tooltip("Center where the note\'s freqs. are turned upside-down");
      o->box(FL_ENGRAVED_FRAME);
      { Fl_Osc_Check* o = new Fl_Osc_Check(254, 13, 55, 30, "Invert keys");
        o->tooltip("Turn upside-down the note frequencies");
        o->box(FL_NO_BOX);
        o->down_box(FL_DOWN_BOX);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_FOREGROUND_COLOR);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(1);
        o->labelsize(11);
        o->labelcolor(FL_FOREGROUND_COLOR);
        o->callback((Fl_Callback*)cb_Invert);
        o->align(Fl_Align(132|FL_ALIGN_INSIDE));
        o->when(FL_WHEN_RELEASE);
        o->init("Pinvertupdown");
      } // Fl_Osc_Check* o
      { Fl_Osc_Counter* o = centerinvertcounter = new Fl_Osc_Counter(319, 13, 80, 20, "Center");
        centerinvertcounter->box(FL_UP_BOX);
        centerinvertcounter->color(FL_BACKGROUND_COLOR);
        centerinvertcounter->selection_color(FL_INACTIVE_COLOR);
        centerinvertcounter->labeltype(FL_NORMAL_LABEL);
        centerinvertcounter->labelfont(1);
        centerinvertcounter->labelsize(11);
        centerinvertcounter->labelcolor(FL_FOREGROUND_COLOR);
        centerinvertcounter->minimum(0);
        centerinvertcounter->maximum(127);
        centerinvertcounter->step(1);
        centerinvertcounter->textfont(1);
        centerinvertcounter->align(Fl_Align(130));
        centerinvertcounter->when(FL_WHEN_CHANGED);
        o->lstep(12/*microtonal->getoctavesize()*/);
        o->init("Pinvertupdowncenter");
      } // Fl_Osc_Counter* centerinvertcounter
      o->end();
    } // Fl_Group* o
    { microtonalgroup = new Fl_Group(3, 49, 402, 398);
      microtonalgroup->box(FL_ENGRAVED_FRAME);
      { applybutton = new Fl_Button(8, 413, 107, 28, "Retune");
        applybutton->tooltip("Retune the synth accorging to the inputs from \"Tunnings\" and \"Keyboard Map\
pings\"");
        applybutton->box(FL_THIN_UP_BOX);
        applybutton->labeltype(FL_EMBOSSED_LABEL);
        applybutton->labelfont(1);
        applybutton->labelsize(13);
        applybutton->callback((Fl_Callback*)cb_applybutton);
      } // Fl_Button* applybutton
      { Fl_Osc_Output* o = octavesizeoutput = new Fl_Osc_Output(150, 423, 35, 17, "nts./oct.");
        octavesizeoutput->tooltip("Notes/Octave");
        octavesizeoutput->box(FL_NO_BOX);
        octavesizeoutput->color(FL_BACKGROUND_COLOR);
        octavesizeoutput->selection_color(FL_BACKGROUND_COLOR);
        octavesizeoutput->labeltype(FL_NORMAL_LABEL);
        octavesizeoutput->labelfont(0);
        octavesizeoutput->labelsize(10);
        octavesizeoutput->labelcolor(FL_FOREGROUND_COLOR);
        octavesizeoutput->maximum(500);
        octavesizeoutput->step(1);
        octavesizeoutput->value(12);
        octavesizeoutput->textfont(1);
        octavesizeoutput->align(Fl_Align(FL_ALIGN_TOP_LEFT));
        octavesizeoutput->when(FL_WHEN_CHANGED);
        o->init("octavesize");
      } // Fl_Osc_Output* octavesizeoutput
      { Fl_Osc_Input* o = nameinput = new Fl_Osc_Input(8, 64, 285, 25, "Name:");
        nameinput->box(FL_DOWN_BOX);
        nameinput->color(FL_BACKGROUND2_COLOR);
        nameinput->selection_color(FL_SELECTION_COLOR);
        nameinput->labeltype(FL_NORMAL_LABEL);
        nameinput->labelfont(1);
        nameinput->labelsize(11);
        nameinput->labelcolor(FL_FOREGROUND_COLOR);
        nameinput->align(Fl_Align(FL_ALIGN_TOP_LEFT));
        nameinput->when(FL_WHEN_RELEASE);
        o->maximum_size(MICROTONAL_MAX_NAME_LEN);
        o->init("Pname");
      } // Fl_Osc_Input* nameinput
      { Fl_Osc_Input* o = tuningsinput = new Fl_Osc_Input(8, 144, 182, 264, "Tunings:");
        tuningsinput->type(4);
        tuningsinput->box(FL_DOWN_BOX);
        tuningsinput->color(FL_BACKGROUND2_COLOR);
        tuningsinput->selection_color(FL_SELECTION_COLOR);
        tuningsinput->labeltype(FL_NORMAL_LABEL);
        tuningsinput->labelfont(1);
        tuningsinput->labelsize(11);
        tuningsinput->labelcolor(FL_FOREGROUND_COLOR);
        tuningsinput->align(Fl_Align(FL_ALIGN_TOP_LEFT));
        tuningsinput->when(FL_WHEN_NEVER);
        o->init("tunings");
      } // Fl_Osc_Input* tuningsinput
      { Fl_Osc_Input* o = commentinput = new Fl_Osc_Input(8, 104, 391, 25, "Comment:");
        commentinput->box(FL_DOWN_BOX);
        commentinput->color(FL_BACKGROUND2_COLOR);
        commentinput->selection_color(FL_SELECTION_COLOR);
        commentinput->labeltype(FL_NORMAL_LABEL);
        commentinput->labelfont(1);
        commentinput->labelsize(11);
        commentinput->labelcolor(FL_FOREGROUND_COLOR);
        commentinput->align(Fl_Align(FL_ALIGN_TOP_LEFT));
        commentinput->when(FL_WHEN_RELEASE);
        o->maximum_size(MICROTONAL_MAX_NAME_LEN);
        o->init("Pcomment");
      } // Fl_Osc_Input* commentinput
      { Fl_Osc_Counter* o = new Fl_Osc_Counter(313, 69, 70, 20, "Shift");
        o->type(1);
        o->box(FL_UP_BOX);
        o->color(FL_BACKGROUND_COLOR);
        o->selection_color(FL_INACTIVE_COLOR);
        o->labeltype(FL_NORMAL_LABEL);
        o->labelfont(0);
        o->labelsize(11);
        o->labelcolor(FL_FOREGROUND_COLOR);
        o->minimum(-64);
        o->maximum(63);
        o->step(1);
        o->textfont(1);
        o->align(Fl_Align(FL_ALIGN_TOP));
        o->when(FL_WHEN_CHANGED);
        o->init("Pscaleshift");
      } // Fl_Osc_Counter* o
      { Fl_Button* o = new Fl_Button(243, 411, 84, 15, "Import .SCL file");
        o->tooltip("Inport Scala .scl file (tunnings)");
        o->box(FL_THIN_UP_BOX);
        o->labelfont(1);
        o->labelsize(10);
        o->callback((Fl_Callback*)cb_Import);
      } // Fl_Button* o
      { keymappinggroup = new Fl_Group(193, 144, 206, 264, "Keyboard Mapping");
        keymappinggroup->box(FL_ENGRAVED_BOX);
        keymappinggroup->labelfont(1);
        keymappinggroup->labelsize(11);
        { Fl_Osc_Input* o = mappinginput = new Fl_Osc_Input(250, 147, 146, 258);
          mappinginput->type(4);
          mappinginput->box(FL_DOWN_BOX);
          mappinginput->color(FL_BACKGROUND2_COLOR);
          mappinginput->selection_color(FL_SELECTION_COLOR);
          mappinginput->labeltype(FL_NORMAL_LABEL);
          mappinginput->labelfont(1);
          mappinginput->labelsize(11);
          mappinginput->labelcolor(FL_FOREGROUND_COLOR);
          mappinginput->align(Fl_Align(FL_ALIGN_TOP_LEFT));
          mappinginput->when(FL_WHEN_NEVER);
          o->init("mapping");
        } // Fl_Osc_Input* mappinginput
        { Fl_Osc_Counter* o = firstnotecounter = new Fl_Osc_Counter(199, 195, 42, 18, "First note");
          firstnotecounter->tooltip("First MIDI note number");
          firstnotecounter->type(1);
          firstnotecounter->box(FL_UP_BOX);
          firstnotecounter->color(FL_BACKGROUND_COLOR);
          firstnotecounter->selection_color(FL_INACTIVE_COLOR);
          firstnotecounter->labeltype(FL_NORMAL_LABEL);
          firstnotecounter->labelfont(0);
          firstnotecounter->labelsize(10);
          firstnotecounter->labelcolor(FL_FOREGROUND_COLOR);
          firstnotecounter->minimum(0);
          firstnotecounter->maximum(127);
          firstnotecounter->step(1);
          firstnotecounter->textfont(1);
          firstnotecounter->textsize(11);
          firstnotecounter->align(Fl_Align(FL_ALIGN_TOP_LEFT));
          firstnotecounter->when(FL_WHEN_CHANGED);
          o->init("Pfirstkey");
        } // Fl_Osc_Counter* firstnotecounter
        { Fl_Osc_Counter* o = lastnotecounter = new Fl_Osc_Counter(199, 225, 42, 18, "Last note");
          lastnotecounter->tooltip("Last MIDI note number");
          lastnotecounter->type(1);
          lastnotecounter->box(FL_UP_BOX);
          lastnotecounter->color(FL_BACKGROUND_COLOR);
          lastnotecounter->selection_color(FL_INACTIVE_COLOR);
          lastnotecounter->labeltype(FL_NORMAL_LABEL);
          lastnotecounter->labelfont(0);
          lastnotecounter->labelsize(10);
          lastnotecounter->labelcolor(FL_FOREGROUND_COLOR);
          lastnotecounter->minimum(0);
          lastnotecounter->maximum(127);
          lastnotecounter->step(1);
          lastnotecounter->value(127);
          lastnotecounter->textfont(1);
          lastnotecounter->textsize(11);
          lastnotecounter->align(Fl_Align(FL_ALIGN_TOP_LEFT));
          lastnotecounter->when(FL_WHEN_CHANGED);
          o->init("Plastkey");
        } // Fl_Osc_Counter* lastnotecounter
        { Fl_Osc_Counter* o = middlenotecounter = new Fl_Osc_Counter(199, 267, 42, 18, "Midle note");
          middlenotecounter->tooltip("Midle note (where scale degree 0 is mapped to)");
          middlenotecounter->type(1);
          middlenotecounter->box(FL_UP_BOX);
          middlenotecounter->color(FL_BACKGROUND_COLOR);
          middlenotecounter->selection_color(FL_INACTIVE_COLOR);
          middlenotecounter->labeltype(FL_NORMAL_LABEL);
          middlenotecounter->labelfont(0);
          middlenotecounter->labelsize(10);
          middlenotecounter->labelcolor(FL_FOREGROUND_COLOR);
          middlenotecounter->minimum(0);
          middlenotecounter->maximum(127);
          middlenotecounter->step(1);
          middlenotecounter->value(60);
          middlenotecounter->textfont(1);
          middlenotecounter->textsize(11);
          middlenotecounter->align(Fl_Align(FL_ALIGN_TOP_LEFT));
          middlenotecounter->when(FL_WHEN_CHANGED);
          o->init("Pmiddlenote");
        } // Fl_Osc_Counter* middlenotecounter
        { Fl_Osc_Output* o = mapsizeoutput = new Fl_Osc_Output(201, 382, 44, 20, "Map Size");
          mapsizeoutput->box(FL_NO_BOX);
          mapsizeoutput->color(FL_BACKGROUND_COLOR);
          mapsizeoutput->selection_color(FL_BACKGROUND_COLOR);
          mapsizeoutput->labeltype(FL_NORMAL_LABEL);
          mapsizeoutput->labelfont(0);
          mapsizeoutput->labelsize(10);
          mapsizeoutput->labelcolor(FL_FOREGROUND_COLOR);
          mapsizeoutput->maximum(500);
          mapsizeoutput->step(1);
          mapsizeoutput->value(12);
          mapsizeoutput->textfont(1);
          mapsizeoutput->align(Fl_Align(FL_ALIGN_TOP_LEFT));
          mapsizeoutput->when(FL_WHEN_CHANGED);
          o->init("Pmapsize");
        } // Fl_Osc_Output* mapsizeoutput
        keymappinggroup->end();
      } // Fl_Group* keymappinggroup
      { Fl_Osc_Check* o = mappingenabledbutton = new Fl_Osc_Check(198, 150, 48, 21, "ON");
        mappingenabledbutton->tooltip("Enable the Mapping (otherwise the mapping is linear)");
        mappingenabledbutton->box(FL_FLAT_BOX);
        mappingenabledbutton->down_box(FL_DOWN_BOX);
        mappingenabledbutton->color(FL_BACKGROUND_COLOR);
        mappingenabledbutton->selection_color(FL_FOREGROUND_COLOR);
        mappingenabledbutton->labeltype(FL_NORMAL_LABEL);
        mappingenabledbutton->labelfont(1);
        mappingenabledbutton->labelsize(14);
        mappingenabledbutton->labelcolor(FL_FOREGROUND_COLOR);
        mappingenabledbutton->callback((Fl_Callback*)cb_mappingenabledbutton);
        mappingenabledbutton->align(Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_INSIDE));
        mappingenabledbutton->when(FL_WHEN_RELEASE);
        o->init("Pmappingenabled");
      } // Fl_Osc_Check* mappingenabledbutton
      { Fl_Button* o = new Fl_Button(243, 428, 84, 16, "Import .kbm file");
        o->tooltip("Inport Scala .kbm file (keyboard mapping)");
        o->box(FL_THIN_UP_BOX);
        o->labelfont(1);
        o->labelsize(10);
        o->callback((Fl_Callback*)cb_Import1);
      } // Fl_Button* o
      microtonalgroup->end();
    } // Fl_Group* microtonalgroup
    { Fl_Group* o = new Fl_Group(108, 2, 140, 45);
      o->box(FL_ENGRAVED_FRAME);
      { Fl_Osc_Counter* o = anotecounter = new Fl_Osc_Counter(173, 17, 65, 20, "\"A\" Note");
        anotecounter->tooltip("The \"A\" note (the reference note for which freq. (\"A\" freq) is given)");
        anotecounter->box(FL_UP_BOX);
        anotecounter->color(FL_BACKGROUND_COLOR);
        anotecounter->selection_color(FL_INACTIVE_COLOR);
        anotecounter->labeltype(FL_NORMAL_LABEL);
        anotecounter->labelfont(1);
        anotecounter->labelsize(10);
        anotecounter->labelcolor(FL_FOREGROUND_COLOR);
        anotecounter->minimum(0);
        anotecounter->maximum(127);
        anotecounter->step(1);
        anotecounter->value(69);
        anotecounter->textfont(1);
        anotecounter->textsize(10);
        anotecounter->callback((Fl_Callback*)cb_anotecounter);
        anotecounter->align(Fl_Align(129));
        anotecounter->when(FL_WHEN_CHANGED);
        o->lstep(12);
        o->init("PAnote");
      } // Fl_Osc_Counter* anotecounter
      { Fl_Osc_Value* o = afreqinput = new Fl_Osc_Value(118, 17, 45, 20, "\"A\" Freq.");
        afreqinput->tooltip("The freq. of \"A\" note (default=440.0)");
        afreqinput->box(FL_DOWN_BOX);
        afreqinput->color(FL_BACKGROUND2_COLOR);
        afreqinput->selection_color(FL_SELECTION_COLOR);
        afreqinput->labeltype(FL_NORMAL_LABEL);
        afreqinput->labelfont(1);
        afreqinput->labelsize(10);
        afreqinput->labelcolor(FL_FOREGROUND_COLOR);
        afreqinput->minimum(1);
        afreqinput->maximum(20000);
        afreqinput->step(0.001);
        afreqinput->value(440);
        afreqinput->textfont(1);
        afreqinput->textsize(10);
        afreqinput->align(Fl_Align(FL_ALIGN_TOP));
        afreqinput->when(FL_WHEN_CHANGED);
        o->init("PAfreq");
      } // Fl_Osc_Value* afreqinput
      o->end();
    } // Fl_Group* o
    { Fl_Button* o = new Fl_Button(333, 413, 67, 28, "Close");
      o->box(FL_THIN_UP_BOX);
      o->callback((Fl_Callback*)cb_Close);
    } // Fl_Button* o
    { Fl_Osc_Check* o = new Fl_Osc_Check(3, 3, 102, 45, "Enable Microtonal");
      o->box(FL_UP_BOX);
      o->down_box(FL_DOWN_BOX);
      o->color(FL_BACKGROUND_COLOR);
      o->selection_color(FL_FOREGROUND_COLOR);
      o->labeltype(FL_NORMAL_LABEL);
      o->labelfont(1);
      o->labelsize(11);
      o->labelcolor(FL_FOREGROUND_COLOR);
      o->callback((Fl_Callback*)cb_Enable);
      o->align(Fl_Align(132|FL_ALIGN_INSIDE));
      o->when(FL_WHEN_RELEASE);
      o->init("Penabled");
    } // Fl_Osc_Check* o
    microtonaluiwindow->end();
  } // Fl_Osc_Window* microtonaluiwindow
  return microtonaluiwindow;
}

void MicrotonalUI::updateTuningsInput() {
  tuningsinput->update();
}

void MicrotonalUI::updateMappingInput() {
  mappinginput->update();
}

MicrotonalUI::MicrotonalUI(Fl_Osc_Interface *osc_, std::string base) {
  osc=osc_;make_window(osc, base);
}

MicrotonalUI::~MicrotonalUI() {
  microtonaluiwindow->hide();
  delete(microtonaluiwindow);
}

void MicrotonalUI::show() {
  microtonaluiwindow->show();
}

void MicrotonalUI::apply() {
  osc->write("/microtonal/tunings", "s", tuningsinput->value());
      osc->write("/microtonal/mapping", "s", mappinginput->value());
      octavesizeoutput->update();
      mapsizeoutput->update();
      anotecounter->update();
}
