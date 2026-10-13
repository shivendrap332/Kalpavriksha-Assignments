#include <stdio.h>

int readNum(char expr[], int *index)
{
    int value = 0;

    while (expr[*index] >= '0' && expr[*index] <= '9')
    {
        value = value * 10 + (expr[*index] - '0');
        (*index)++;
    }

    return value;
}

int calculate(char expr[], int *result)
{
    char lastOp = '+';
    int total = 0;
    int current = 0;
    int needNum = 1;
    int seenNum = 0;

    for (int i = 0; expr[i] != '\0';)
    {
        char ch = expr[i];

        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
        {
            i++;
            continue;
        }

        if (ch >= '0' && ch <= '9')
        {
            if (!needNum)
                return 0;

            int value = readNum(expr, &i);

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
            {
                current *= value;
            }
            else if (lastOp == '/')
            {
                if (value == 0)
                    return -1;

                current /= value;
            }

            needNum = 0;
            seenNum = 1;
        }
        else if (ch == '+' || ch == '-' ||
                 ch == '*' || ch == '/')
        {
            if (needNum)
                return 0;

            lastOp = ch;
            needNum = 1;
            i++;
        }
        else
        {
            return 0;
        }
    }

    if (!seenNum || needNum)
        return 0;

    total += current;
    *result = total;

    return 1;
}

int main()
{
    char expr[1000];
    int result;
    int status;

    fgets(expr, sizeof(expr), stdin);

    status = calculate(expr, &result);

    if (status == -1)
    {
        printf("Error: Division by zero.\n");
    }
    else if (status == 0)
    {
        printf("Error: Invalid expression.\n");
    }
    else
    {
        printf("%d\n", result);
    }

    return 0;
}
