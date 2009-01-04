// Gmsh - Copyright (C) 1997-2009 C. Geuzaine, J.-F. Remacle
//
// See the LICENSE.txt file for license information. Please report all
// bugs and problems to <gmsh@geuz.org>.

#include <sstream>
#include <FL/Fl_Input.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Value_Input.H>
#include "GUI.h"
#include "Draw.h"
#include "fieldWindow.h"
#include "dialogWindow.h"
#include "fileDialogs.h"
#include "GmshDefines.h"
#include "GModel.h"
#include "PView.h"
#include "GmshMessage.h"
#include "Field.h"
#include "GeoStringInterface.h"
#include "Options.h"
#include "Context.h"

extern Context_T CTX;

void field_cb(Fl_Widget *w, void *data)
{
  GUI::instance()->fields->win->show();
  GUI::instance()->fields->editField(NULL);
}

static void field_delete_cb(Fl_Widget *w, void *data)
{
  Field *f = (Field*)GUI::instance()->fields->editor_group->user_data();
  delete_field(f->id, GModel::current()->getFileName());
  GUI::instance()->fields->editField(NULL);
}

static void field_new_cb(Fl_Widget *w, void *data)
{
  Fl_Menu_Button* mb = ((Fl_Menu_Button*)w);
  FieldManager *fields = GModel::current()->getFields();
  int id = fields->new_id();
  add_field(id, mb->text(), GModel::current()->getFileName());
  GUI::instance()->fields->editField((*fields)[id]);
}

static void field_apply_cb(Fl_Widget *w, void *data)
{
  GUI::instance()->fields->saveFieldOptions();
}

static void field_revert_cb(Fl_Widget *w, void *data)
{
  GUI::instance()->fields->loadFieldOptions();
}

static void field_browser_cb(Fl_Widget *w, void *data)
{
  int selected = GUI::instance()->fields->browser->value();
  if(!selected){
    GUI::instance()->fields->editField(NULL);
  }
  Field *f = (Field*)GUI::instance()->fields->browser->data(selected);
  GUI::instance()->fields->editField(f);
}

static void field_put_on_view_cb(Fl_Widget *w, void *data)
{
  Fl_Menu_Button* mb = ((Fl_Menu_Button*)w);
  Field *field = (Field*)GUI::instance()->fields->editor_group->user_data();
  int iView;
  if(sscanf(mb->text(), "View [%i]", &iView)){
    if(iView < (int)PView::list.size()){
      field->put_on_view(PView::list[iView]);
    }
  }
  else{
    field->put_on_new_view();
    GUI::instance()->updateViews();
  }
  Draw();
}

static void field_select_file_cb(Fl_Widget *w, void *data)
{
  Fl_Input *input = (Fl_Input*)data;
  int ret = file_chooser(0, 0, "File selection", "", input->value());
  if(ret){
    input->value(file_chooser_get_name(0).c_str());
    input->set_changed();
  }
}

static void field_select_node_cb(Fl_Widget *w, void *data)
{
  const char *mode = "select";
  const char *help = "vertices";
  CTX.pick_elements = 1;
  Draw();  
  opt_geometry_points(0, GMSH_SET | GMSH_GUI, 1);
  while(1) {
    Msg::StatusBar(3, false, "Select %s\n[Press %s'q' to abort]", 
                   help, mode ? "" : "'u' to undo or ");
    char ib = GUI::instance()->selectEntity(ENT_POINT);
    printf("char = %c\n", ib);
    if(ib == 'q'){
      for(std::vector<GVertex*>::iterator it = GUI::instance()->selectedVertices.begin();
          it != GUI::instance()->selectedVertices.end(); it++){
	printf("%p\n", *it);
      }
      break;
    }
  }
  CTX.mesh.changed = ENT_ALL;
  CTX.pick_elements = 0;
  Msg::StatusBar(3, false, "");
  Draw();  
}

fieldWindow::fieldWindow(int deltaFontSize) : _deltaFontSize(deltaFontSize)
{
  FL_NORMAL_SIZE -= deltaFontSize;

  int width0 = 34 * FL_NORMAL_SIZE + WB;
  int height0 = 13 * BH + 5 * WB;
  int width = (CTX.field_size[0] < width0) ? width0 : CTX.field_size[0];
  int height = (CTX.field_size[1] < height0) ? height0 : CTX.field_size[1];

  win = new dialogWindow(width, height, CTX.non_modal_windows, "Fields");
  win->box(GMSH_WINDOW_BOX);

  int x = WB, y = WB, w = (int)(1.5 * BB), h = height - 2 * WB - 3 * BH;

  Fl_Menu_Button *new_btn = new Fl_Menu_Button(x, y, w, BH, "New");
  FieldManager &fields = *GModel::current()->getFields();

  std::map<std::string, FieldFactory*>::iterator it;
  for(it = fields.map_type_name.begin(); it != fields.map_type_name.end(); it++)
    new_btn->add(it->first.c_str());
  new_btn->callback(field_new_cb);

  y += BH;
  browser = new Fl_Hold_Browser(x, y + WB, w, h - 2 * WB);
  browser->callback(field_browser_cb);

  y += h; 
  delete_btn = new Fl_Button(x, y, w, BH, "Delete");
  delete_btn->callback(field_delete_cb, this);

  y += BH;
  put_on_view_btn = new Fl_Menu_Button(x, y, w, BH, "Put on view");
  put_on_view_btn->callback(field_put_on_view_cb, this);

  x += w + WB;
  y = WB;
  w = width - x - WB;
  h = height - y - WB;
  editor_group = new Fl_Group(x, y, w, h);
    
  title = new Fl_Box(x, y, w, BH, "field_name");
  title->labelfont(FL_BOLD);
  title->labelsize(18);
  
  y += BH + WB;
  h -= BH + WB;
  Fl_Tabs *tabs = new Fl_Tabs(x, y , w, h);
  y += BH;
  h -= BH;
  x += WB;
  w -= 2 * WB;

  Fl_Group *options_tab = new Fl_Group(x, y, w, h, "Options");
  
  options_scroll = new Fl_Scroll(x, y, w, h - BH - 2 * WB);
  options_scroll->end();
  
  Fl_Button *apply_btn = new Fl_Return_Button
    (x + w - BB, y + h - BH - WB, BB, BH, "Apply");
  apply_btn->callback(field_apply_cb, this);
  
  Fl_Button *revert_btn = new Fl_Button
    (x + w - 2 * BB - WB, y + h - BH - WB, BB, BH, "Revert");
  revert_btn->callback(field_revert_cb, this);
  
  background_btn = new Fl_Check_Button
    (x, y + h - BH - WB, (int)(1.5 * BB), BH, "Background mesh size");
  options_tab->end();

  Fl_Group *help_tab = new Fl_Group(x, y, w, h, "Help");
  help_display = new Fl_Browser(x, y + WB, w, h - 2 * WB);
  help_tab->end();

  tabs->end();

  editor_group->end();

  win->resizable(new Fl_Box((int)(1.5 * BB) + 2 * WB, BH + 2 * WB, 
                            width - 3 * WB - (int)(1.5 * BB),
                            height - 3 * BH - 5 * WB));
  editor_group->resizable(tabs);
  tabs->resizable(options_tab);
  options_tab->resizable(new Fl_Box(3 * BB + 4 * WB, BH + 2 * WB,
                                    width - 9 * WB - 5 * BB, 
                                    height - 3 * BH - 5 * WB));
  win->size_range(width0, height0);
  win->position(CTX.field_position[0], CTX.field_position[1]);
  win->end();

  FL_NORMAL_SIZE += deltaFontSize;

  loadFieldViewList();
  editField(NULL);
}

void fieldWindow::loadFieldViewList()
{
  put_on_view_btn->clear();
  put_on_view_btn->add("New view");
  put_on_view_btn->activate();
  for(unsigned int i = 0; i < PView::list.size(); i++) {
    std::ostringstream s;
    s << "View [" << i << "]";
    put_on_view_btn->add(s.str().c_str());
  }
}

void fieldWindow::loadFieldList()
{
  FieldManager &fields = *GModel::current()->getFields();
  Field *selected_field = (Field*)editor_group->user_data();
  browser->clear();
  int i_entry = 0;
  for(FieldManager::iterator it = fields.begin(); it != fields.end(); it++){
    i_entry++;
    Field *field = it->second;
    std::ostringstream sstream;
    if(it->first == fields.background_field)
      sstream << "@b";
    sstream << it->first << " " << field->get_name();
    browser->add(sstream.str().c_str(), field);
    if(it->second == selected_field)
      browser->select(i_entry);
  }
}

void fieldWindow::saveFieldOptions()
{
  std::list<Fl_Widget*>::iterator input = options_widget.begin();
  Field *f = (Field*)editor_group->user_data();
  std::ostringstream sstream;
  int i;
  char a;
  sstream.precision(16);
  for(std::map<std::string, FieldOption*>::iterator it = f->options.begin();
      it != f->options.end(); it++){
    FieldOption *option = it->second;
    sstream.str("");
    switch(option->get_type()){
    case FIELD_OPTION_STRING:
    case FIELD_OPTION_PATH:
      sstream << "\"" << ((Fl_Input*)*input)->value() << "\"";
      break;
    case FIELD_OPTION_INT:
      sstream << (int)((Fl_Value_Input*)*input)->value();
      break;
    case FIELD_OPTION_DOUBLE:
      sstream << ((Fl_Value_Input*)*input)->value();
      break;
    case FIELD_OPTION_BOOL:
      sstream << (bool)((Fl_Check_Button*)*input)->value();
      break;
    case FIELD_OPTION_LIST:
      {
        sstream << "{";
        std::istringstream istream(((Fl_Input*)*input)->value());
        while(istream >> i){
          sstream << i;
          if(istream >> a){
            if(a != ',')
              Msg::Error("Unexpected character \'%c\' while parsing option "
                         "'%s' of field \'%s\'", a, it->first.c_str(), f->id);
            sstream << ", ";
          }
        }
        sstream << "}";
      }
      break;
    }
    if((*input)->changed()){
      add_field_option(f->id, it->first.c_str(), sstream.str().c_str(), 
                       GModel::current()->getFileName());
      (*input)->clear_changed();
    }
    input++;
  }
  int is_bg_field = background_btn->value();
  FieldManager &fields = *GModel::current()->getFields();
  if(is_bg_field && fields.background_field != f->id){
    set_background_field(f->id, GModel::current()->getFileName());
    loadFieldList();
  }
  if(!is_bg_field && fields.background_field == f->id){
    set_background_field(-1, GModel::current()->getFileName());
    loadFieldList();
  }
}

void fieldWindow::loadFieldOptions()
{
  Field *f = (Field*)editor_group->user_data();
  std::list<Fl_Widget*>::iterator input = options_widget.begin();
  for(std::map<std::string, FieldOption*>::iterator it = f->options.begin();
      it != f->options.end(); it++){
    FieldOption *option = it->second;
    std::ostringstream vstr;
    std::list<int>::iterator list_it;
    switch(option->get_type()){
    case FIELD_OPTION_STRING:
    case FIELD_OPTION_PATH:
      ((Fl_Input*)(*input))->value(option->string().c_str());
      break;
    case FIELD_OPTION_INT:
    case FIELD_OPTION_DOUBLE:
      ((Fl_Value_Input*)(*input))->value(option->numerical_value());
      break;
    case FIELD_OPTION_BOOL:
      ((Fl_Check_Button*)(*input))->value((int)option->numerical_value());
      break;
    case FIELD_OPTION_LIST:
      vstr.str("");
      for(list_it = option->list().begin(); list_it != option->list().end();
          list_it++){
	if(list_it!=option->list().begin())
	  vstr << ", ";
	vstr << *list_it;
      }
      ((Fl_Input*)(*input))->value(vstr.str().c_str());
      break;
    }
    (*input)->clear_changed();
    input++;
  }
  background_btn->value(GModel::current()->getFields()->background_field == f->id);
}

void fieldWindow::editField(Field *f)
{
  editor_group->user_data(f);
  put_on_view_btn->deactivate();
  delete_btn->deactivate();
  if(f == NULL){
    selected_id = -1;
    editor_group->hide();
    loadFieldList();
    return;
  }

  FL_NORMAL_SIZE -= _deltaFontSize;

  selected_id = f->id;
  editor_group->show();
  editor_group->user_data(f);
  title->label(f->get_name());
  options_scroll->clear();
  options_widget.clear();
  options_scroll->begin();
  int x = options_scroll->x();
  int yy = options_scroll->y() + WB;
  help_display->clear();
  add_multiline_in_browser(help_display, "", f->get_description().c_str(), 100);
  help_display->add("\n");
  help_display->add("@b@cOptions");
  for(std::map<std::string, FieldOption*>::iterator it = f->options.begin(); 
      it != f->options.end(); it++){
    Fl_Widget *input;
    help_display->add(("@b" + it->first).c_str());
    help_display->add(("@i" + it->second->get_type_name()).c_str());
    add_multiline_in_browser
      (help_display, "", it->second->get_description().c_str(), 100);
    switch(it->second->get_type()){
    case FIELD_OPTION_INT:
    case FIELD_OPTION_DOUBLE:
      input = new Fl_Value_Input(x, yy, IW, BH, it->first.c_str());
      break;
    case FIELD_OPTION_BOOL:
      input = new Fl_Check_Button(x, yy, BH, BH, it->first.c_str());
      break;
    case FIELD_OPTION_PATH:
      {
        Fl_Button *b = new Fl_Button(x, yy, BH, BH, "S");
        input = new Fl_Input(x + WB + BH, yy, IW - WB - BH, BH, it->first.c_str());
        b->callback(field_select_file_cb, input);
      }
      break;
    case FIELD_OPTION_STRING:
      input = new Fl_Input(x, yy, IW, BH, it->first.c_str());
      break;
    case FIELD_OPTION_LIST:
    default:
      input = new Fl_Input(x, yy, IW, BH, it->first.c_str());
      break;
    }
    input->align(FL_ALIGN_RIGHT);
    options_widget.push_back(input);
    yy += WB + BH;
  }
  options_scroll->end();

  FL_NORMAL_SIZE += _deltaFontSize;

  loadFieldOptions();
  options_scroll->damage(1);
  put_on_view_btn->activate();
  delete_btn->activate();
  loadFieldList();
}
