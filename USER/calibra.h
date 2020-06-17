#ifndef _CALIBRA_H_
#define _CALIBRA_H_


#define LINE_X_STA_NO		0x00
#define LINE_X_STA_T_LEFT	0x01		//×ó±ß×ß£¬T×ÖÐÎ
#define LINE_X_STA_L_LEFT	0x02	
#define LINE_X_STA_T_RIGHT	0x03
#define LINE_X_STA_L_RIGHT	0x04

#define LINE_X_STA_T_HIGHT	0x01		//Y+
#define LINE_X_STA_L_HIGHT	0x02		//Y+
#define LINE_X_STA_T_LOW	0x03		//Y-
#define LINE_X_STA_L_LOW	0x04		//Y-


#define MAP_LINE_NO		0x00
#define MAP_LINE_T		0x01
#define MAP_LINE_L		0x02



int calibrat_opera(navigat_t *navi,short bx,short by,uint16_t gsta);
int unclr_piont_walk(uint8_t old_side);
uint8_t ajust_ofset_y(int16_t x,int16_t y,int16_t bx1,int16_t ex1,uint8_t check_len,int16_t walk_len,int16_t x_dir);

int calibrat_opera(navigat_t *navi,short bx,short by,uint16_t gsta);
uint8_t obst_line_90_270_light(int16_t tx,int16_t ty,int16_t y_ofs_90,int16_t y_ofs_270);
uint8_t check_line_x_sta(int16_t ox,int16_t oy,int16_t len,int16_t y_dir);
uint8_t check_line_y_sta(int16_t ox,int16_t oy,int16_t len,int16_t x_dir);
uint8_t walk_adj_y(int16_t tox,int16_t toy,int16_t dir,int16_t ox,int16_t oy);
uint8_t walk_adj_x(int16_t tox,int16_t toy,int16_t dir,int16_t ox,int16_t oy);
uint8_t ajust_y_from_x(int16_t tx,int16_t ty);
int16_t calibrat_map(int16_t tx,int16_t ty);


#endif
