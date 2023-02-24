#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "datatypes.h"
#include "llist.h"

void list_init (LIST *l)
{
    l->pHead = NULL;
    l->pTail = NULL;
}

NODE *list_node (elements x) // Khởi tạo 1 cái node
{
    NODE *item = (NODE *)malloc(sizeof(NODE)); // cấp phát vùng nhớ cho NODE p
    if (item == NULL)
    {
        printf("Not enough memory to allocate.");
        return NULL;
    }
    
    item->data = x; // truyền giá trị x vào cho data
    item->pNext = NULL; // đầu tiên khai báo node thì node chưa có liên kết đến node nào hết => con trỏ sẽ trỏ đến NULL
    return item; // trả về NODE p vừa khởi tạo
}

void list_add (LIST *l, NODE *item) // Thêm node p vào đầu
{
    if (item == NULL) // Nếu node thêm vào rỗng
        printf("Item is empty.");
    else
        if (l->pHead == NULL) // Danh sách đang rỗng
            l->pHead = l->pTail = item; // node  đầu là node cuối là p
        else
        {
            item->pNext = l->pHead; // cho con trỏ của node cần thêm là node p liên kết đến node đầu - pHead
            l->pHead = item;
        }
}

void list_push (LIST *l, NODE *item) // Thêm node p vào cuối
{
    if (item == NULL) // Nếu node thêm vào rỗng
    {
        printf("Item is empty.");
    }
    else
    {
        if (l->pHead == NULL) // Danh sách đang rỗng
        {
            l->pHead = l->pTail = item; // node đầu là node cuối là p
        }
        else
        {
            (l->pTail)->pNext = item; // cho con trỏ của node pTail liên kết với node cần thêm là p
            item->pNext = NULL; // cập nhật lại p là node pTail
            l->pTail = item;
        }
    }
}

NODE *list_pop (LIST *l) // Xóa node đầu
{
    if (l->pHead == NULL)
        return NULL;
    
    NODE *item = l->pHead; // node p là node sẽ xóa
    l->pHead = item->pNext; // cập nhật lại l->pHead là phần tử kế tiếp
    free(item); // Giải phóng bộ nhớ
    return item;
}

NODE *list_chop (LIST *l) // Xóa node cuối
{
    if (l->pHead == NULL)
        return NULL;

    NODE *k, *item;
    item = l->pTail;

    for (k = l->pTail; k->pNext->pNext != NULL; k = k->pNext);

    k->pNext = NULL;
    l->pTail = k;
    return item;
}

void list_remove (LIST *l, NODE *item) // Xóa phần tử cụ thể
{
    if (l->pHead == NULL)
        printf("Linked list is empty");

    if (item == l->pHead)
        list_pop(l);

    for (NODE *k = l->pHead; k != NULL; k = k->pNext)
        if (k->pNext == item)
        {
            NODE *r = k->pNext;
            k->pNext = r->pNext;
            free(r);
        }
}

int list_length (LIST *l) // Lấy độ dài
{
    int n = 0;

    for (NODE *k = l->pHead; k != NULL; k = k->pNext)
        ++n;

    return n;    
}

int list_empty (LIST *l) // Kiểm tra rỗng
{
    return (l->pHead == NULL);
}

void list_insert(LIST *l, NODE *item, int position) // Thêm node p vào vị trí x (bắt đầu từ 0)
{
    int n = list_length(l); // Lấy độ dài linked list

    // Danh sách đang rỗng hoặc thêm ở vị trí 0
    if(l->pHead == NULL || position == 0)
    {
        list_add(l, item);
    }
    else if(position >= n) // Nếu vị trí thêm >= độ dài linked list
    {                    
        list_push(l, item);
    }
    else
    {
        n = 0;
        for(NODE *k = l->pHead; k != NULL; k = k->pNext)
        {
            if(n == position)
            {
                item->pNext = k;
                break;
            }
            n++;
        }

        n = 0;
        for(NODE *k = l->pHead; k != NULL; k = k->pNext)
        {
            if(n == position - 1)
            {
                k->pNext = item;
                break;
            }
            n++;
        }
    }
}

NODE *list_remove_x(LIST *l, int x) // Hàm xóa node ở vị trí x
{
    NODE *r = NULL;
    if (l->pHead == NULL)
        printf("Linked list is empty");

    if(x == 0) // Nếu x = 0 thì xóa vị trí đầu
        list_pop(l);
    else
    {
        NODE *p = l->pHead;
        for(int k = 0; k < x - 1; k++)
        {
            p = p->pNext;
        }
        NODE *q = p->pNext->pNext;
        r = p->pNext;
        //free(p->pNext);
        p->pNext = q;
    }

    return r;
}

void deleteList(LIST *l) // Hàm xóa List
{
    if(l->pHead != NULL)
    {
        list_pop(l);
        deleteList(l);
    }
}

NODE* searchByUsername(LIST *l, char user[])// Hàm tìm kiếm theo Username
{
    char temp1[1000], temp2[1000];
    strcpy(temp1, user);
    for (NODE *k = l->pHead; k != NULL; k = k->pNext)
    {
        strcpy(temp2, k->data.username);
        if(strcmp(temp2, temp1) == 0)
            return k;
    }

    return NULL;
}