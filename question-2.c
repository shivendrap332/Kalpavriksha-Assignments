#include <stdio.h>

typedef struct {
    int id;
    char name[50];
    int age;
} User;

void createFile(){
    FILE *f = fopen("users.txt","a");
    if(f == NULL){
        printf("Error creating file.\n");
        return;
    }
    fclose(f);
}

void addUser()
{
    User newU, old;
    printf("\n\tEnter ID Name Age: ");
    scanf("%d %49s %d",&newU.id, newU.name,&newU.age);

    FILE *f = fopen("users.txt", "r");
    if (f == NULL) {
        printf("Error opening file.\n");
        return;
    }
    while(fscanf(f,"%d %49s %d", &old.id, old.name, &old.age) == 3)
    {
        if(old.id == newU.id){
            printf("ID already exists.\n");
            fclose(f);
            return;
        }
    }
    fclose(f);


    f = fopen("users.txt","a");
    if(f == NULL) {
        printf("Error opening file.\n");
        return;
    }
    fprintf(f, "%d %s %d\n",newU.id,newU.name,newU.age);
    fclose(f);
    printf("User added successfully.\n");
}

void displayUsers(){
    User usr;
    FILE *f = fopen("users.txt","r");

    if(f == NULL)
    {
        printf("No users found.\n");
        return;
    }

    printf("\nID Name Age\n");
    while(fscanf(f, "%d %49s %d", &usr.id, usr.name, &usr.age)==3)
        printf("%d %s %d\n", usr.id, usr.name, usr.age);

    fclose(f);
}

void updateUser()
{
    int uid, flag = 0;
    User rec;

    printf("Enter ID to update: ");
    scanf("%d", &uid);

    FILE *f = fopen("users.txt", "r");
    if(f == NULL){
        printf("File not found.\n");
        return;
    }

    FILE *tf = fopen("temp.txt","w");
    if (tf == NULL) {
        printf("Error creating temporary file.\n");
        fclose(f);
        return;
    }

    while(fscanf(f,"%d %49s %d",&rec.id,rec.name,&rec.age) == 3){
        if(rec.id == uid){
            flag = 1;
           printf("\n\tEnter new Name Age: ");
            scanf("%49s %d", rec.name, &rec.age);
        }
        fprintf(tf, "%d %s %d\n", rec.id, rec.name, rec.age);
    }

    fclose(f);  fclose(tf);

    remove("users.txt");
    rename("temp.txt","users.txt");

    if(flag) printf("User updated successfully.\n");
    else printf("User ID not found.\n");
}

void deleteUser(){
    int delId, ok = 0;
    User rec;

    printf("Enter ID to delete: ");
    scanf("%d",&delId);

    FILE *fin = fopen("users.txt","r");
    if (fin == NULL)
    {
        printf("File not found.\n");
        return;
    }

    FILE *fout = fopen("temp.txt", "w");
    if(fout == NULL){
        printf("Error creating temporary file.\n");
        fclose(fin);
        return;
    }

    while(fscanf(fin, "%d %49s %d", &rec.id, rec.name, &rec.age) == 3)
    {
        if(rec.id == delId){
            ok = 1;
            continue;
        }
        fprintf(fout,"%d %s %d\n",rec.id,rec.name,rec.age);
    }

    fclose(fin);
    fclose(fout);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (ok)
        printf("User deleted successfully.\n");
    else
        printf("User ID not found.\n");
}

int main(){
    int ch;
    createFile();

    do {
        printf("\n1. Create\n");
        printf("2. Read\n");
        printf("3. Update\n");
        printf("4. Delete\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d",&ch);

        if(ch == 1) addUser();
        else if(ch == 2) displayUsers();
        else if(ch == 3) updateUser();
        else if(ch == 4) deleteUser();
        else if(ch != 5) printf("Invalid choice.\n");

    } while(ch != 5);

    return 0;
}

