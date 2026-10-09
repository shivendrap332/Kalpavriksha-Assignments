#include <stdio.h>

struct Student {
    int roll;
    char name[100];
    int m1, m2, m3;
};

int passMark = 35;

int calculateTotal(struct Student s) {
    return s.m1 + s.m2 + s.m3;
}

float calculateAverage(int totalMarks) {
    return totalMarks / 3.0;
}

char calculateGrade(float avg) {

    if(avg >= 85)
        return 'A';

    else if(avg >= 70)
        return 'B';

    else if(avg >= 50)
        return 'C';

    else if(avg >= 35)
        return 'D';

    else
        return 'F';
}

void printrollnums(struct Student s[], int i, int n) {

    if(i >= n)
        return;

    printf(" %d", s[i].roll);

    printrollnums(s, i + 1, n);
}

int main() {

    int n;
    struct Student s[100];
    scanf("%d", &n);

    for(int i = 0; i < n; i++) {
        scanf("%d %s %d %d %d",
              &s[i].roll, s[i].name,
              &s[i].m1, &s[i].m2, &s[i].m3);
    }

    for(int i = 0; i < n; i++) {

        int totalMarks = calculateTotal(s[i]);
        float avg = calculateAverage(totalMarks);
        char g = calculateGrade(avg);

        printf("Roll: %d\n", s[i].roll);
        printf("Name: %s\n", s[i].name);
        printf("Total: %d\n", totalMarks);
        printf("Average: %.2f\n", avg);
        printf("Grade: %c\n", g);

         if(avg < passMark) {
          printf("\n");
         continue;
         }

        int stars;

        if(g == 'A') {
            stars = 5;
        }
        else if(g == 'B') {
            stars = 4;
        }
        else if(g == 'C') {
            stars = 3;
        }
        else {
            stars = 2;
        }

        printf("Performance: ");

        for(int j = 0; j < stars; j++) {
            printf("*");
        }

        printf("\n");
    }

    printf("List of Roll Numbers (via recursion):");

    printrollnums(s, 0, n);
   printf("\n");

    return 0;
}

