#ifndef FILE_DATA_TYPE
#define FILE_DATA_TYPE

#define BUFF_SIZE 1024

// KHAI BÁO CẤU TRÚC DATA
struct Image
{
    char filename[BUFF_SIZE];
    char path[BUFF_SIZE];
};

typedef struct Image IMAGE;

typedef IMAGE elements;

// Khai báo cấu trúc 1 cái node
typedef struct node
{
    elements data; // dữ liệu chứa trong 1 cái node
    struct node *pNext; // con trỏ dùng để liên kết các node với nhau.
}NODE; // thay thế 'struct node' bằng 'NODE'. C++ không cần 'struct node', còn C thì cần.

// Khai báo cấu trúc của danh sách liên kết đơn
typedef struct list
{
    NODE *pHead; // node quản lý đầu danh sách
    NODE *pTail; // node quản lý cuối danh sách
}LIST; // thay thế 'struct list' thành 'LIST'

#endif