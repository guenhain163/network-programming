#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "datatypes.h"
#include "llist.h"

#define ACTIVED 1
#define BLOCKED 0

// KHAI BÁO CÁC HÀM
void inputData(); // Nhập data từ file

char *strlwr(char *s);

void clear(); //

// PRINT MENU
void menuPrint();

void Display(LIST l);

void inputKey(LIST *l);

void outputData(USER u); // Hàm ghi thông tin sách vào file

NODE *signin(LIST *l); // Hàm đăng nhập

void searchInfo(LIST *l, NODE *login); // Tìm kiếm thông tin tài khoản

void signOut(LIST *l, NODE *login); // Đăng xuất

/********** MAIN **********/
int main()
{
    LIST l;
    list_init(&l);
    NODE *login = NULL;

    inputData(&l);
    int choice = 0;
    do
    {
        menuPrint();
        printf("Input choice: ");

    RETURN:
        scanf("%d", &choice);
        switch (choice)
        {
        case 1:
            // Nhập data từ bàn phím
            inputKey(&l);
            break;
        case 2:
            login = signin(&l);
            break;
        case 3:
            searchInfo(&l, login);
            break;
        case 4:
            signOut(&l, login);
            break;
        case 5:
            deleteList(&l);
            exit(0);
            break;
        default:
            menuPrint();
            printf("You entered it wrong, please re-enter!\n");
            printf("Input choice: ");
            goto RETURN;
            break;
        }
    } while (choice > 0 && choice <= 5);

    deleteList(&l);
    return 0;
}

char *strlwr(char *s)
{
    char *tmp = s;

    for (; *tmp; ++tmp)
    {
        *tmp = tolower((unsigned char)*tmp);
    }

    return s;
}

void inputData(LIST *l) // Nhập data từ file
{
    FILE *fin;
    fin = fopen("account.txt", "r");

    if (!fin)
    {
        printf("File Error!");
    }
    else
    {
        while (!feof(fin))
        {
            USER *temp = (USER *)malloc(sizeof(USER));
            fscanf(fin, "%[^ ] %[^ ] %d\n\n", temp->username, temp->password, &temp->status);
            list_push(l, list_node(*temp));
            free(temp);
        }
    }

    fclose(fin);
}

void clear()
{
    int c;

    while ((c = getchar()) != '\n' && c != EOF)
    {
    }
}

void menuPrint()
{
    printf("\nUSER MANAGEMENT PROGRAM\n");
    printf("-----------------------------------\n");
    printf("1. Register\n");
    printf("2. Sign in\n");
    printf("3. Search\n");
    printf("4. Sign out\n");
    printf("5. Exit\n\n");
}

// Hàm xuất danh sách liên kết đơn
void Display(LIST l)
{
    // int i = 0;
    printf("\nLIST ACCOUNTS:\n");
    printf("%-50s %-50s %-10s\n", "USERNAME", "PASSWORD", "ACCIVE");
    printf("--------------------------------------------------------------------------------------------------------------\n");
    for (NODE *k = l.pHead; k != NULL; k = k->pNext)
    {
        printf("%-50s %-50s %-10d\n", k->data.username, k->data.password, k->data.status);
    }
}

// Hàm nhập thông tin sách từ bàn phím
void inputKey(LIST *l)
{
    USER *temp = (USER *)malloc(sizeof(USER));
    NODE *checkUser;

    printf("Username: ");
    clear();
    scanf("%[^\n]s", temp->username);

    checkUser = searchByUsername(l, temp->username);

    if (checkUser != NULL)
    {
        free(temp);
        printf("Account existed\n");
        return;
    }

    printf("Password: ");
    clear();
    scanf("%[^\n]s", temp->password);
    temp->status = ACTIVED;

    list_push(l, list_node(*temp));
    outputData(*temp);
    printf("Successful registration\n");
    // Display(*l);
    free(temp);
}

// Hàm ghi thông tin sách vào file
void outputData(USER u)
{
    FILE *fin = fopen("account.txt", "a");

    if (!fin)
    {
        printf("File loi!");
    }
    else
    {
        fprintf(fin, "%s %s %d\n\n", u.username, u.password, u.status);
    }

    fclose(fin);
}

void outputAllData(LIST l)
{
    FILE *fin = fopen("account.txt", "wb");

    if (!fin)
    {
        printf("File loi!");
    }
    else
    {
        for (NODE *k = l.pHead; k != NULL; k = k->pNext)
        {
            fprintf(fin, "%s %s %d\n\n", k->data.username, k->data.password, k->data.status);
        }
    }

    fclose(fin);
}

NODE *signin(LIST *l) // Hàm đăng nhập
{
    int timesLogin = 0;
    USER *temp = (USER *)malloc(sizeof(USER));
    NODE *checkUser;
    char again[100];

AGAIN_SIGNIN:
    printf("\nUsername: ");
    clear();
    scanf("%[^\n]s", temp->username);

    checkUser = searchByUsername(l, temp->username);
    if (checkUser == NULL)
    {
        do
        {
            printf("Cannot find account\n");
            printf("Do you want signin again? (y/n)\n");
            clear();
            scanf("%[^\n]s", again);

            if (strcmp(strlwr(again), "yes") == 0 || strcmp(strlwr(again), "y") == 0)
            {
                goto AGAIN_SIGNIN;
            }
            else if (strcmp(strlwr(again), "no") == 0 || strcmp(strlwr(again), "n") == 0)
            {
                free(temp);
                return NULL;
            }
        } while (strcmp(strlwr(again), "yes") != 0 || strcmp(strlwr(again), "y") != 0 || strcmp(strlwr(again), "no") != 0 || strcmp(strlwr(again), "n") != 0);
    }
    else
    {
        if (checkUser->data.status == BLOCKED)
        {
            printf("Account is blocked\n");
            free(temp);
            return NULL;
        }

    AGAIN_INPUT_PASSWORD:
        timesLogin++;
        printf("Password: ");
        clear();
        scanf("%[^\n]s", temp->password);

        if (strcmp(temp->password, checkUser->data.password) == 0)
        {
            if (checkUser->data.status == ACTIVED)
            {
                printf("SUCCESSFULLY SIGNIN!\nHello %s\n", checkUser->data.username);
                free(temp);
                return checkUser;
            }
            else
            {
                printf("YOUR ACCOUNT HAS BEEN BLOCKED!\n");
                free(temp);
                return NULL;
            }
        }

        if (timesLogin < 3)
        {
            printf("Password is incorrect\n");
            do
            {
                printf("Do you want input password again? (Y/n)\n");
                clear();
                scanf("%[^\n]s", again);
                again[strlen(again)] = '\0';
                if (strcmp(strlwr(again), "yes") == 0 || strcmp(strlwr(again), "y") == 0)
                {
                    goto AGAIN_INPUT_PASSWORD;
                }
                if (strcmp(strlwr(again), "no") == 0 || strcmp(strlwr(again), "n") == 0)
                {
                    free(temp);
                    return NULL;
                }
            } while (strcmp(strlwr(again), "yes") != 0 || strcmp(strlwr(again), "y") != 0 || strcmp(strlwr(again), "no") != 0 || strcmp(strlwr(again), "n") != 0);
        }
        else
        {
            checkUser->data.status = BLOCKED;
            printf("Password is incorrect. Account is blocked\n");
            outputAllData(*l);
        }
    }

    free(temp);
    return NULL;
}

void searchInfo(LIST *l, NODE *login) // Tìm kiếm thông tin tài khoản
{
    NODE *checkUser;
    if (login == NULL)
    {
        printf("You are not logged in\n");
        return;
    }
    checkUser = searchByUsername(l, login->data.username);
    if (checkUser == NULL || strcmp(checkUser->data.password, login->data.password) != 0)
    {
        printf("You are not logged in\n");
    }
    else if (checkUser->data.status == BLOCKED)
    {
        printf("Your account has been blocked\n");
    }
    else
    {
        char user[100];
        printf("Username: ");
        clear();
        scanf("%[^\n]s", user);
        checkUser = searchByUsername(l, user);
        if (checkUser == NULL)
        {
            printf("Cannot find account\n");
        }
        else
        {
            if (checkUser->data.status == ACTIVED)
            {
                printf("%s is active\n", user);
            }
            else
            {
                printf("%s has been blocked\n", user);
            }
        }
    }
}

void signOut(LIST *l, NODE *login) // Đăng xuất
{
    if (login == NULL)
    {
        printf("Account is not sign in\n");
        return;
    }
    else
    {
        char user[100];
        NODE *checkUser = NULL;
        printf("Username: ");
        clear();
        scanf("%[^\n]s", user);
        checkUser = searchByUsername(l, user);

        if (checkUser == NULL)
        {
            printf("Cannot find account\n");
            return;
        }
        else if (strcmp(checkUser->data.username, login->data.username) == 0)
        {
            login = NULL;
            printf("Goodbye %s\n", user);
        } else {
            printf("Account is not sign in\n");
            return;
        }
    }
}