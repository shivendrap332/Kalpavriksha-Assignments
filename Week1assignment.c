
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
    if (avg >= 85)
        return 'A';
    else if (avg >= 70)
        return 'B';
    else if (avg >= 50)
        return 'C';
    else if (avg >= 35)
        return 'D';
    else
        return 'F';
}

void printrollnums(struct Student s[], int i, int n) {
    if (i >= n)
        return;

    printf(" %d", s[i].roll);
    printrollnums(s, i + 1, n);
}

int main(void) {
    int n;
    struct Student s[100];

    printf("Enter number of students: ");
    if (scanf("%d", &n) != 1 || n < 1 || n > 100) {
        printf("Error: Number of students must be between 1 and 100.\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        printf("Enter roll number, name, and marks in 3 subjects for student %d: ", i + 1);

        if (scanf("%d %99s %d %d %d",
                  &s[i].roll, s[i].name,
                  &s[i].m1, &s[i].m2, &s[i].m3) != 5) {
            printf("Error: Invalid input.\n");
            return 1;
        }

        if (s[i].m1 < 0 || s[i].m1 > 100 ||
            s[i].m2 < 0 || s[i].m2 > 100 ||
            s[i].m3 < 0 || s[i].m3 > 100) {
            printf("Error: Marks must be between 0 and 100.\n");
            return 1;
        }
    }

    for (int i = 0; i < n; i++) {
        int totalMarks = calculateTotal(s[i]);
        float avg = calculateAverage(totalMarks);
        char g = calculateGrade(avg);

        printf("Roll: %d\n", s[i].roll);
        printf("Name: %s\n", s[i].name);
        printf("Total: %d\n", totalMarks);
        printf("Average: %.2f\n", avg);
        printf("Grade: %c\n", g);

        if (avg < passMark) {
            printf("\n");
            continue;
        }

        int stars;

        if (g == 'A')
            stars = 5;
        else if (g == 'B')
            stars = 4;
        else if (g == 'C')
            stars = 3;
        else
            stars = 2;

        printf("Performance: ");

        for (int j = 0; j < stars; j++) {
            printf("*");
        }

        printf("\n");
    }

    printf("List of Roll Numbers (via recursion):");
    printrollnums(s, 0, n);
    printf("\n");

    return 0;
}
