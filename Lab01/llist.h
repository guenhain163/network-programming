#ifndef __LLIST_H__
#define __LLIST_H__
#include "datatypes.h"

void list_init (LIST *l); // Khởi tạo

NODE *list_node (elements x); // Khởi tạo 1 cái node

void list_add (LIST *l, NODE *item); // Thêm node p vào đầu

void list_push (LIST *l, NODE *item); // Thêm node p vào cuối

NODE *list_pop (LIST *l); // Xóa node đầu

NODE *list_chop (LIST *l); // Xóa node cuối

void list_remove (LIST *l, NODE *item); // Xóa phần tử cụ thể

int list_length (LIST *l); // Lấy độ dài

int list_empty (LIST *l); // Kiểm tra rỗng

void list_insert(LIST *l, NODE *item, int position); // Thêm node p vào vị trí position

NODE *list_remove_x(LIST *l, int x); // Hàm xóa node ở vị trí x

void deleteList(LIST *l); // Hàm xóa List

NODE* searchByUsername(LIST *l, char user[]); // Hàm tìm kiếm theo Username

// Các hàm trong chương trình cụ thể
/**
 * 
void Display(LIST l); // Hàm xuất danh sách liên kết đơn

int Search(LIST *l, int x); // Hàm tìm kiếm 
**/

#endif