
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}

static void generateMatrix(int *m, int n)
{
    int *p = m;
    int *end = m + n * n;

    while (p < end)
    {
        *p++ = rand() % 256;
    }
}

static void printMatrix(const int *m, int n)
{
    const int *p = m;

    for (int r = 0; r < n; r++)
    {
        for (int c = 0; c < n; c++)
        {
            printf("%4d", *p++);
        }
        printf("\n");
    }
}

static void rotateClockwise(int *m, int n)
{
    for (int r = 0; r < n; r++)
    {
        for (int c = r + 1; c < n; c++)
        {
            swap(m + r * n + c, m + c * n + r);
        }
    }

    int *row = m;

    for (int r = 0; r < n; r++)
    {
        int *left = row;
        int *right = row + n - 1;

        while (left < right)
        {
            swap(left, right);
            left++;
            right--;
        }

        row += n;
    }
}

static void smoothFilter(int *m, int n)
{
    int *prev = malloc(n * sizeof(int));
    int *curr = malloc(n * sizeof(int));

    if (prev == NULL || curr == NULL)
    {
        printf("Memory allocation failed\n");
        free(prev);
        free(curr);
        exit(1);
    }

    int *row = m;

    for (int r = 0; r < n; r++)
    {
        const int *src = row;
        int *dst = curr;

        for (int c = 0; c < n; c++)
        {
            *dst++ = *src++;
        }

        int *cell = row;

        for (int c = 0; c < n; c++)
        {
            int sum = 0;
            int count = 0;

            for (int dr = -1; dr <= 1; dr++)
            {
                int rr = r + dr;

                if (rr < 0 || rr >= n)
                    continue;

                const int *base;

                if (dr == -1)
                    base = prev;
                else if (dr == 0)
                    base = curr;
                else
                    base = row + n;

                for (int dc = -1; dc <= 1; dc++)
                {
                    int cc = c + dc;

                    if (cc < 0 || cc >= n)
                        continue;

                    sum += *(base + cc);
                    count++;
                }
            }

            *cell++ = sum / count;
        }

        int *temp = prev;
        prev = curr;
        curr = temp;

        row += n;
    }

    free(prev);
    free(curr);
}

int main(void)
{
    int n;

    printf("Enter matrix size (2-10): ");

    if (scanf("%d", &n) != 1 || n < 2 || n > 10)
    {
        printf("Invalid size. Must be between 2 and 10.\n");
        return 1;
    }

    srand((unsigned int)time(NULL));

    int *matrix = malloc(n * n * sizeof(int));

    if (matrix == NULL)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    generateMatrix(matrix, n);

    printf("\nOriginal Randomly Generated Matrix:\n");
    printMatrix(matrix, n);

    rotateClockwise(matrix, n);

    printf("\nMatrix after 90 degree Clockwise Rotation:\n");
    printMatrix(matrix, n);

    smoothFilter(matrix, n);

    printf("\nMatrix after Applying 3x3 Smoothing Filter:\n");
    printMatrix(matrix, n);

    free(matrix);

    return 0;
}
