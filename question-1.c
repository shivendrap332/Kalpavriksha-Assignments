#include <stdio.h>

void calculate()
{
    char expr[1000];
    char lastOp = '+';
    int total = 0;
    int current = 0;
    
    int needNum = 1;
    int seenNum = 0;
printf("Enter the expression: ");
    fgets(expr, sizeof(expr), stdin);

    for (int i = 0; expr[i] != '\0'; i++)
    {
        char ch = expr[i];

        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
            continue;

        if (ch >= '0' && ch <= '9')
        {
            if (!needNum)
            {
                printf("Error: Invalid expression.\n");
                return;
            }

            int value = 0;

            while (expr[i] >= '0' && expr[i] <= '9')
            {
                value = value * 10 + (expr[i] - '0');
                i++;
            }

            i--;

            if (lastOp == '+')
            {
                total += current;
                current = value;
            }
            else if (lastOp == '-')
            {
                total += current;
                current = -value;
            }
            else if (lastOp == '*')
                current *= value;
            else if (lastOp == '/')
            {
                if (value == 0)
                {
                    printf("Error: Division by zero.\n");
                    return;
                }

                current /= value;
            }

            needNum = 0;
            seenNum = 1;
        }
        else if (ch == '+' || ch == '-' || ch == '*' || ch == '/')
        {
            if (needNum)
            {
                printf("Error: Invalid expression.\n");
                return;
            }

            lastOp = ch;
            needNum = 1;
        }
        else
        {
            printf("Error: Invalid expression.\n");
            return;
        }
    }

    if (!seenNum || needNum)
    {
        printf("Error: Invalid expression.\n");
        return;
    }

    total += current;
    printf("%d\n", total);
}

int main()
{
    calculate();
    return 0;
}
