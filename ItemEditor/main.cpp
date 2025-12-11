/*
- want to separate a KO item from a KO mesh to allow for easy swapping
*/

#include "defines.h"

#include "base.h"
#include "ItemInfo.h"
#include "GLItemViewer.h"

const int _gl_width = 380-60;
const int _gl_height = 1024/4;

//-----------------------------------------------------------------------------
extern e_ItemType eType;

//-----------------------------------------------------------------------------
class GLItemViewer* m_sw;
Fl_Choice* choice;

//int num_disp_files;
//char** disp_files;

Fl_Int_Input* tbl_id;
Fl_Int_Input* tbl_ext_index;
Fl_Input*     tbl_name;
Fl_Input*     tbl_remark;

ItemInfo* item_being_rendered;

int num_races = 10;

e_Race races[] = {
	RACE_ALL,
	RACE_KA_ARKTUAREK,
	RACE_KA_TUAREK,
	RACE_KA_WRINKLETUAREK,
	RACE_KA_PURITUAREK,
	RACE_EL_BABARIAN,
	RACE_EL_MAN,
	RACE_EL_WOMEN,
	RACE_NPC,
	RACE_UNKNOWN
};

char* race_names[] = {
	"RACE_ALL",
	"RACE_KA_ARKTUAREK",
	"RACE_KA_TUAREK",
	"RACE_KA_WRINKLETUAREK",
	"RACE_KA_PURITUAREK",
	"RACE_EL_BABARIAN",
	"RACE_EL_MAN",
	"RACE_EL_WOMEN",
	"RACE_NPC",
	"RACE_UNKNOWN"
};

//-----------------------------------------------------------------------------
//CSTLMap<_ITEM_TABLE> ItemTableMap;
//CN3TableBase<__TABLE_ITEM_BASIC>* s_pTbl_Items_Basic;

//-----------------------------------------------------------------------------
class ItemTableView: public Fl_Table_Row {
private:
	Fl_Color cell_bgcolor;
	Fl_Color cell_fgcolor;

protected:
	void event_callback_update_opengl(void);
	static void event_callback(Fl_Widget* widget, void* data);
	void draw_cell(TableContext context,
		int r = 0, int c = 0, int x = 0, int y = 0, int w = 0, int h = 0
	);

public:
	Fl_Color GetCellBGColor(void) const {return cell_bgcolor;}
	Fl_Color GetCellFGColor(void) const {return cell_fgcolor;}

	void SetCellBGColor(Fl_Color color) {cell_bgcolor = color;}
	void SetCellFGColor(Fl_Color color) {cell_fgcolor = color;}

public:
	ItemTableView(int x, int y, int w, int h, const char* l = nullptr):
	Fl_Table_Row(x, y, w, h, l) {
		cell_bgcolor = FL_WHITE;
		cell_fgcolor = FL_BLACK;
		callback(&event_callback, (void*)this);
		end();
	}
};

void ItemTableView::event_callback(Fl_Widget* widget, void* data) {
	ItemTableView* item_table = (ItemTableView*)data;
	item_table->event_callback_update_opengl();
}

// TEMP
extern int                m_nFC;
extern __VertexXyzNormal* m_pVertices;
extern uint16_t*              m_pwVtxIndices;
extern float*             m_pfUVs;
extern uint16_t*              m_pwUVsIndices;

// TEMP
extern _N3TexHeader    HeaderOrg;
extern uint8_t*  compTexData;
extern int             compTexSize;
extern int             iPixelSize;
extern uint16_t* m_pIndices0;
extern _N3VertexT1*    m_pVertices0;
extern int             m_iMaxNumIndices0;
extern int             m_iMaxNumVertices0;

void ItemTableView::event_callback_update_opengl(void) {
	int r = callback_row();
	int c = callback_col();
	TableContext context = callback_context();
	if(r==-1 || c==-1) return;
	if((int)Fl::event()!=1 && Fl::event()!=FL_KEYDOWN) return;

	for(int i=0; i<ItemInfo::GetNumTblItems(); ++i) {
		if(row_selected(i)) select_row(i, 0);
	}

	select_row(r);

	// NOTE: remove all the races and only add the ones we have stuff for
	choice->clear();

	int first_choice = -1;
	int first_pos_race = num_races;

	item_being_rendered = ItemInfo::GetItem(r);
	e_ItemType type = item_being_rendered->getItemType();
	if(type==ITEM_TYPE_PART) { //type==ITEM_TYPE_PLUG || 
		for(int i=1; i<num_races-1; ++i) { // -1 so not to include RACE_UNKNOWN and i=0 because none should be race all?...
			if(item_being_rendered->getItemMeshFileForRace(races[i]) != "") {
				int j = choice->add(race_names[i]);
				if(i < first_pos_race) {
					first_choice = j;
					first_pos_race = i;
				}
			}
		}
	} else {
		if(item_being_rendered->getItemMeshFileForRace(races[0]) != "") {
			int j = choice->add(race_names[0]);
			if(0 < first_pos_race) {
				first_choice = j;
				first_pos_race = 0;
			}
		}
	}

	set_visible_focus();

	if(first_choice == -1) {
		m_sw->Clear();
		choice->clear();
		choice->add("RACE_UNKNOWN");
		choice->value(0);
		choice->clear_visible_focus();
	} else {
		choice->value(first_choice);
		choice->clear_visible_focus();
		m_sw->RenderItem(item_being_rendered, races[first_pos_race]);
	}
}

void ItemTableView::draw_cell(TableContext context,
	int r, int c, int x, int y, int w, int h
) {
	static char s[(NAME_LENGTH+1)];
	memset(s, 0x00, (NAME_LENGTH+1)*sizeof(char));

	_ITEM_TABLE* item = nullptr;
	__TABLE_ITEM_BASIC* item_tbl = nullptr;

	ItemInfo* item_info;

	switch(context) {
		case CONTEXT_STARTPAGE:
			fl_font(FL_HELVETICA, 16);
			return;
		case CONTEXT_COL_HEADER:
			switch(c) {
				case 0: strcpy(s, "dwID");     break;
				case 1: strcpy(s, "szName"); break;
				case 2: strcpy(s, "szRemark"); break;
			}

			fl_push_clip(x, y, w, h);
			{
				fl_draw_box(FL_THIN_UP_BOX, x, y, w, h, col_header_color());
				fl_color(FL_BLACK);
				fl_draw(s, x, y, w, h, FL_ALIGN_CENTER);
			}
			fl_pop_clip();
			return;
		case CONTEXT_ROW_HEADER:
			sprintf(s, "%d", r+1);

			fl_push_clip(x, y, w, h);
			{
				fl_draw_box(FL_THIN_UP_BOX, x, y, w, h, row_header_color());
				fl_color(FL_BLACK);
				fl_draw(s, x, y, w, h, FL_ALIGN_CENTER);
			}
			fl_pop_clip();
			return;
		case CONTEXT_CELL: {
			// NOTE: need to find a way to get the rows for the same item
			item_info = ItemInfo::GetItem(r);
			//item_tbl = ItemInfo::_tbl_item_info->GetIndexedData(r);

			switch(c) {
				case 0: sprintf(s, "  %d", item_info->tbl_info.dwID);  break;
				case 1: sprintf(s, "  %s", item_info->tbl_info.szName.c_str()); break;
				case 2: sprintf(s, "  %s", item_info->tbl_info.szRemark.c_str()); break;
			}

			fl_push_clip(x, y, w, h);
			{
				fl_color(row_selected(r) ? selection_color() : cell_bgcolor);
				fl_rectf(x, y, w, h);
				fl_color(cell_fgcolor);
				fl_draw(s, x, y, w, h, FL_ALIGN_LEFT);
				fl_color(color());
				fl_rect(x, y, w, h);
			}
			fl_pop_clip();
		} return;
		case CONTEXT_TABLE:
			printf("Table Context Called?\n");
			return;
		case CONTEXT_ENDPAGE:
		case CONTEXT_RC_RESIZE:
		case CONTEXT_NONE:
			return;
	}
}

//-----------------------------------------------------------------------------
Fl_Menu_Item menu_table[] = {
	{"&File",0,0,0,FL_SUBMENU},
		{"&Open",	FL_ALT+'o', 0, 0, FL_MENU_INACTIVE},
		{"&Close",	0,	0},
		{"&Quit",	FL_ALT+'q', 0, 0, FL_MENU_DIVIDER},
		{"shortcut",'a'},
		{"shortcut",FL_SHIFT+'a'},
		{"shortcut",FL_CTRL+'a'},
		{"shortcut",FL_CTRL+FL_SHIFT+'a'},
		{"shortcut",FL_ALT+'a'},
		{"shortcut",FL_ALT+FL_SHIFT+'a'},
		{"shortcut",FL_ALT+FL_CTRL+'a'},
		{"shortcut",FL_ALT+FL_SHIFT+FL_CTRL+'a', 0,0, FL_MENU_DIVIDER},
		{"shortcut",'\r'/*FL_Enter*/},
		{"shortcut",FL_CTRL+FL_Enter, 0,0, FL_MENU_DIVIDER},
		{"shortcut",FL_F+1},
		{"shortcut",FL_SHIFT+FL_F+1},
		{"shortcut",FL_CTRL+FL_F+1},
		{"shortcut",FL_SHIFT+FL_CTRL+FL_F+1},
		{"shortcut",FL_ALT+FL_F+1},
		{"shortcut",FL_ALT+FL_SHIFT+FL_F+1},
		{"shortcut",FL_ALT+FL_CTRL+FL_F+1},
		{"shortcut",FL_ALT+FL_SHIFT+FL_CTRL+FL_F+1, 0,0, FL_MENU_DIVIDER},
		{"&Submenus", FL_ALT+'S',	0, (void*)"Submenu1", FL_SUBMENU},
			{"A very long menu item"},
			{"&submenu",FL_CTRL+'S',	0, (void*)"submenu2", FL_SUBMENU},
				{"item 1"},
				{"item 2"},
				{"item 3"},
				{"item 4"},
			{0},
			{"after submenu"},
		{0},
	{0},
	{"&Edit",FL_F+2,0,0,FL_SUBMENU},
		{"Undo",	FL_ALT+'z',	0},
		{"Redo",	FL_ALT+'r',	0, 0, FL_MENU_DIVIDER},
		{"Cut",	FL_ALT+'x',	0},
		{"Copy",	FL_ALT+'c',	0},
		{"Paste",	FL_ALT+'v',	0},
		{"Inactive",FL_ALT+'d',	0, 0, FL_MENU_INACTIVE},
		{"Clear",	0,	0, 0, FL_MENU_DIVIDER},
		{"Invisible",FL_ALT+'e',	0, 0, FL_MENU_INVISIBLE},
		{"Preferences",0,	0},
		{"Size",	0,	0},
	{0},
	{"&Checkbox",FL_F+3,0,0,FL_SUBMENU},
		{"&Alpha",	FL_F+2,	0, (void *)1, FL_MENU_TOGGLE},
		{"&Beta",	0,	0, (void *)2, FL_MENU_TOGGLE},
		{"&Gamma",	0,	0, (void *)3, FL_MENU_TOGGLE},
		{"&Delta",	0,	0, (void *)4, FL_MENU_TOGGLE|FL_MENU_VALUE},
		{"&Epsilon",0,	0, (void *)5, FL_MENU_TOGGLE},
		{"&Pi",	0,	0, (void *)6, FL_MENU_TOGGLE},
		{"&Mu",	0,	0, (void *)7, FL_MENU_TOGGLE|FL_MENU_DIVIDER},
		{"Red",	0,	0, (void *)1, FL_MENU_TOGGLE, 0, 0, 0, 1},
		{"Black",	0,	0, (void *)1, FL_MENU_TOGGLE|FL_MENU_DIVIDER},
		{"00",	0,	0, (void *)1, FL_MENU_TOGGLE},
		{"000",	0,	0, (void *)1, FL_MENU_TOGGLE},
	{0},
	{"&Radio",0,0,0,FL_SUBMENU},
		{"&Alpha",	0,	0, (void *)1, FL_MENU_RADIO},
		{"&Beta",	0,	0, (void *)2, FL_MENU_RADIO},
		{"&Gamma",	0,	0, (void *)3, FL_MENU_RADIO},
		{"&Delta",	0,	0, (void *)4, FL_MENU_RADIO|FL_MENU_VALUE},
		{"&Epsilon",0,	0, (void *)5, FL_MENU_RADIO},
		{"&Pi",	0,	0, (void *)6, FL_MENU_RADIO},
		{"&Mu",	0,	0, (void *)7, FL_MENU_RADIO|FL_MENU_DIVIDER},
		{"Red",	0,	0, (void *)1, FL_MENU_RADIO},
		{"Black",	0,	0, (void *)1, FL_MENU_RADIO|FL_MENU_DIVIDER},
		{"00",	0,	0, (void *)1, FL_MENU_RADIO},
		{"000",	0,	0, (void *)1, FL_MENU_RADIO},
	{0},
	{"&Font",0,0,0,FL_SUBMENU /*, 0, FL_BOLD, 20*/},
		{"Normal",	0, 0, 0, 0, 0, 0, 14},
		{"Bold",	0, 0, 0, 0, 0, FL_BOLD, 14},
		{"Italic",	0, 0, 0, 0, 0, FL_ITALIC, 14},
		{"BoldItalic",0,0,0, 0, 0, FL_BOLD+FL_ITALIC, 14},
		{"Small",	0, 0, 0, 0, 0, FL_BOLD+FL_ITALIC, 10},
		{"Emboss",	0, 0, 0, 0, (uchar)FL_EMBOSSED_LABEL},
		{"Engrave",	0, 0, 0, 0, (uchar)FL_ENGRAVED_LABEL},
		{"Shadow",	0, 0, 0, 0, (uchar)FL_SHADOW_LABEL},
		{"@->",	0, 0, 0, 0, (uchar)FL_SYMBOL_LABEL},
	{0},
	{"&International",0,0,0,FL_SUBMENU},
		{"Sharp Ess",0x0000df},
		{"A Umlaut",0x0000c4},
		{"a Umlaut",0x0000e4},
		{"Euro currency",FL_COMMAND+0x0020ac},
		{"the &\xc3\xbc Umlaut"},  // &uuml;
		{"the capital &\xc3\x9c"}, // &Uuml;
		{"convert \xc2\xa5 to &\xc2\xa3"}, // Yen to GBP
		{"convert \xc2\xa5 to &\xe2\x82\xac"}, // Yen to Euro
		{"Hangul character Sios &\xe3\x85\x85"},
		{"Hangul character Cieuc", 0x003148},
	{0},
	{"E&mpty",0,0,0,FL_SUBMENU},
	{0},
	{"&Inactive", 0,	0, 0, FL_MENU_INACTIVE|FL_SUBMENU},
		{"A very long menu item"},
		{"A very long menu item"},
	{0},
	{"Invisible",0,	0, 0, FL_MENU_INVISIBLE|FL_SUBMENU},
		{"A very long menu item"},
		{"A very long menu item"},
	{0},
	{"button",FL_F+4, 0, 0, FL_MENU_TOGGLE},
	{0}
};

//-----------------------------------------------------------------------------
void test_cb(Fl_Widget* w, void*) {
	Fl_Menu_* mw = (Fl_Menu_*)w;
	const Fl_Menu_Item* m = mw->mvalue();
	if (!m)
		printf("NULL\n");
	else if (m->shortcut())
		printf("%s - %s\n", m->label(), fl_shortcut_label(m->shortcut()));
	else
		printf("%s\n", m->label());
}

//-----------------------------------------------------------------------------
void choice_cb(Fl_Widget* w, void*) {
	Fl_Choice* c = (Fl_Choice*)w;
	int choice_i = c->value();

	int pick = num_races-1;
	for(int i=0; i<num_races; ++i) {
		if(!strcmp(c->text(choice_i), race_names[i])) {
			pick = i;
		}
	}

	m_sw->RenderItem(item_being_rendered, races[pick]);
}

//-----------------------------------------------------------------------------
void mesh_cb(Fl_Widget* w, void*) {
	char file_ext[0xFF] = {};
	if(item_being_rendered->getItemType() == ITEM_TYPE_PLUG) {
		strcpy(file_ext, "*.n3cplug");
	} else if(item_being_rendered->getItemType() == ITEM_TYPE_PART) {
		strcpy(file_ext, "*.n3cpart");
	}

	Fl_File_Chooser chooser("./item", file_ext, Fl_File_Chooser::SINGLE, "test");
	chooser.show();

	while(chooser.shown()) {Fl::wait();}

	if(chooser.value() == nullptr) {
		printf("Nothing selected\n");
		return;
	}

	fprintf(stderr, "--------------------\n");
	fprintf(stderr, "DIRECTORY: '%s'\n", chooser.directory());
	fprintf(stderr, "    VALUE: '%s'\n", chooser.value());
	fprintf(stderr, "    COUNT: %d files selected\n", chooser.count());

	if(chooser.count() > 1) {
		for(int t=0; t<chooser.count(); ++t) {
			fprintf(stderr, " VALUE[%d]: '%s'\n", t, chooser.value(t));
		}
	}

	char tmp[0xFFF] = {};
	strcpy(tmp, chooser.value());
	int start = static_cast<int>(strlen(tmp));
	for(; start>=0; --start) {
		if(tmp[start] == '/') break;
	}
	
	start++;

	printf("filename: %s\n", &tmp[start]);

	int choice_i = choice->value();
	int pick = num_races-1;
	for(int i=0; i<num_races; ++i) {
		if(!strcmp(choice->text(choice_i), race_names[i])) {
			pick = i;
		}
	}

	item_being_rendered->setItemMeshFileForRace(races[pick], string(&tmp[start]));
	m_sw->RenderItem(item_being_rendered, races[pick]);
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv) {
	//----

	if(ItemInfo::LoadInformation() == false) {
		printf("ERROR: unable to load item information.\n");
		system("pause");
		return -1;
	}

	ItemInfo::CreateItemsFromInfo();

	//----

	Fl::use_high_res_GL(true);
	Fl_Window window(1024, 720, "KO Item Editor");

	GLItemViewer sw(window.w()-_gl_width, 30, _gl_width, _gl_height);
	m_sw = &sw;

	choice = new Fl_Choice(window.w()-_gl_width, 30+_gl_height, _gl_width/2+50, 25);
	choice->callback(choice_cb);

	Fl_Button button(window.w()-_gl_width+_gl_width/2+50, 30+_gl_height, _gl_width-(_gl_width/2+50), 25, "Update Mesh");
	button.callback(mesh_cb);

	ItemTableView demo_table(0, 30, window.w()-(_gl_width+0), _gl_height);
	demo_table.selection_color(FL_YELLOW);
	demo_table.when(FL_WHEN_RELEASE|FL_WHEN_CHANGED);
	demo_table.table_box(FL_NO_BOX);
	demo_table.col_resize_min(4);
	demo_table.row_resize_min(4);

	demo_table.row_header(true);
	demo_table.row_header_width(60);
	demo_table.row_resize(true);
	demo_table.rows(ItemInfo::GetNumTblItems());
	demo_table.row_height_all(20);

	demo_table.col_header(true);
	demo_table.col_header_height(25);
	demo_table.col_resize(true);
	demo_table.cols(3);
	demo_table.col_width_all(150);

	Fl_Menu_Bar menubar(0, 0, window.w(), 30);
	menubar.menu(menu_table);
	menubar.callback(test_cb);

	// TODO: need to add the positions and widths/heights as variables
	/*
	Fl_Tabs info_tabs(0, _gl_height+40, window.w(), window.h()-_gl_height-30-10);
		Fl_Group group(0, _gl_height+40+35, window.w(), window.h()-_gl_height-30-10, "Table Info");
			tbl_id        = new Fl_Int_Input(80, _gl_height+30+35+20+5, 240, 30, "ID:");
			tbl_ext_index = new Fl_Int_Input(80, _gl_height+30+35+20+40+5, 240, 30, "Ext Index:");
			tbl_name      = new Fl_Input    (80, _gl_height+30+35+20+80+5, 240, 30, "Name:");
			tbl_remark    = new Fl_Input    (80, _gl_height+30+35+20+120+5, 240, 30, "Remark:");
		group.end();
		Fl_Group group2(0, _gl_height+40+35, window.w(), window.h()-_gl_height-30-10, "Database Info");
			Fl_Input input2(10, _gl_height+30+35+20, 240, 30);
		group2.end();
	info_tabs.end();
	*/

	window.end();
	window.show(argc, argv);

	sw.show();
	sw.redraw_overlay();

	return Fl::run();
}
