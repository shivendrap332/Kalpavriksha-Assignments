#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAME_SIZE 50
#define DESIGNATION_SIZE 50
#define INPUT_SIZE 128
#define NO_MANAGER 0

typedef struct Employee Employee;

typedef struct EmployeeList
{
    Employee **items;
    int count;
    int capacity;
} EmployeeList;

struct Employee
{
    int id;
    char name[NAME_SIZE];
    char designation[DESIGNATION_SIZE];

    Employee *manager;
    EmployeeList reports;
};

EmployeeList topLevelEmployees = {NULL, 0, 0};

int listAdd(EmployeeList *list, Employee *employee)
{
    if (list->count == list->capacity)
    {
        int newCapacity = (list->capacity == 0) ? 2 : list->capacity * 2;

        Employee **newItems = realloc(
            list->items, newCapacity * sizeof(Employee *)
        );

        if (newItems == NULL)
        {
            return 0;
        }

        list->items = newItems;
        list->capacity = newCapacity;
    }

    list->items[list->count] = employee;
    list->count++;

    return 1;
}

int listReserve(EmployeeList *list, int required)
{
    Employee **newItems;
    int newCapacity;

    if (list->capacity >= required)
    {
        return 1;
    }

    newCapacity = (list->capacity == 0) ? 2 : list->capacity;

    while (newCapacity < required)
    {
        newCapacity *= 2;
    }

    newItems = realloc(list->items, newCapacity * sizeof(Employee *));

    if (newItems == NULL)
    {
        return 0;
    }

    list->items = newItems;
    list->capacity = newCapacity;

    return 1;
}

void listRemove(EmployeeList *list, Employee *employee)
{
    int i;

    for (i = 0; i < list->count; i++)
    {
        if (list->items[i] == employee)
        {
            for (; i < list->count - 1; i++)
            {
                list->items[i] = list->items[i + 1];
            }

            list->count--;
            return;
        }
    }
}

void trimInput(char *text)
{
    size_t length = strlen(text);
    size_t start = 0;

    while (length > 0 && isspace((unsigned char)text[length - 1]))
    {
        text[--length] = '\0';
    }

    while (text[start] != '\0' && isspace((unsigned char)text[start]))
    {
        start++;
    }

    if (start > 0)
    {
        memmove(text, text + start, strlen(text + start) + 1);
    }
}

void readLine(const char *prompt, char *input, size_t size)
{
    printf("%s", prompt);

    if (fgets(input, (int)size, stdin) == NULL)
    {
        input[0] = '\0';
        return;
    }

    if (strchr(input, '\n') == NULL)
    {
        int ch;

        while ((ch = getchar()) != '\n' && ch != EOF)
        {
        }
    }

    trimInput(input);
}

int readInteger(const char *prompt, int *number)
{
    char input[INPUT_SIZE];
    char *end;
    long value;

    readLine(prompt, input, sizeof(input));

    if (input[0] == '\0')
    {
        return 0;
    }

    value = strtol(input, &end, 10);

    if (*end != '\0' || value < 0 || value > 2000000000L)
    {
        return 0;
    }

    *number = (int)value;
    return 1;
}

Employee *findInSubtree(Employee *employee, int searchId)
{
    int i;
    Employee *found;

    if (employee->id == searchId)
    {
        return employee;
    }

    for (i = 0; i < employee->reports.count; i++)
    {
        found = findInSubtree(employee->reports.items[i], searchId);

        if (found != NULL)
        {
            return found;
        }
    }

    return NULL;
}

Employee *findEmployee(int searchId)
{
    int i;
    Employee *found;

    for (i = 0; i < topLevelEmployees.count; i++)
    {
        found = findInSubtree(topLevelEmployees.items[i], searchId);

        if (found != NULL)
        {
            return found;
        }
    }

    return NULL;
}

int countReports(const Employee *employee)
{
    int i;
    int total = 0;

    for (i = 0; i < employee->reports.count; i++)
    {
        total += 1 + countReports(employee->reports.items[i]);
    }

    return total;
}

void displayHierarchy(const Employee *employee, int level)
{
    int i;

    printf("%*s%d - %s - %s\n",
           level * 4, "",
           employee->id,
           employee->name,
           employee->designation);

    for (i = 0; i < employee->reports.count; i++)
    {
        displayHierarchy(employee->reports.items[i], level + 1);
    }
}

void freeEmployeeTree(Employee *employee)
{
    int i;

    for (i = 0; i < employee->reports.count; i++)
    {
        freeEmployeeTree(employee->reports.items[i]);
    }

    free(employee->reports.items);
    free(employee);
}

void addEmployee(void)
{
    int employeeId, managerId;

    char employeeName[NAME_SIZE];
    char designation[DESIGNATION_SIZE];

    Employee *manager = NULL;
    Employee *newEmployee;
    EmployeeList *destination;

    if (!readInteger("Enter Employee ID (positive number): ", &employeeId)
        || employeeId == 0)
    {
        printf("Error: Employee ID must be a positive number.\n");
        return;
    }

    if (findEmployee(employeeId) != NULL)
    {
        printf("Error: Employee ID %d already exists.\n", employeeId);
        return;
    }

    readLine("Enter Name: ", employeeName, sizeof(employeeName));

    if (employeeName[0] == '\0')
    {
        printf("Error: Name cannot be empty.\n");
        return;
    }

    readLine("Enter Designation: ", designation, sizeof(designation));

    if (designation[0] == '\0')
    {
        printf("Error: Designation cannot be empty.\n");
        return;
    }

    if (!readInteger("Enter Reporting Manager ID (0 if top-level): ",
                     &managerId))
    {
        printf("Error: Invalid Manager ID.\n");
        return;
    }

    if (managerId == employeeId)
    {
        printf("Error: An employee cannot report to themselves.\n");
        return;
    }

    if (managerId != NO_MANAGER)
    {
        manager = findEmployee(managerId);

        if (manager == NULL)
        {
            printf("Error: Manager ID %d does not exist.\n", managerId);
            return;
        }
    }

    newEmployee = calloc(1, sizeof(Employee));

    if (newEmployee == NULL)
    {
        printf("Error: Not enough memory.\n");
        return;
    }

    newEmployee->id = employeeId;

    strcpy(newEmployee->name, employeeName);
    strcpy(newEmployee->designation, designation);

    newEmployee->manager = manager;

    destination = (manager == NULL)
                  ? &topLevelEmployees
                  : &manager->reports;

    if (!listAdd(destination, newEmployee))
    {
        printf("Error: Not enough memory.\n");
        free(newEmployee);
        return;
    }

    printf("Employee %d added successfully.\n", employeeId);
}

void removeEmployee(void)
{
    int employeeId;
    int i;

    Employee *employee;
    Employee *newManager;
    EmployeeList *destination;

    if (!readInteger("Enter Employee ID to remove: ", &employeeId))
    {
        printf("Error: Invalid Employee ID.\n");
        return;
    }

    employee = findEmployee(employeeId);

    if (employee == NULL)
    {
        printf("Error: Employee ID %d not found.\n", employeeId);
        return;
    }

    newManager = employee->manager;

    destination = (newManager == NULL)
                  ? &topLevelEmployees
                  : &newManager->reports;

    if (!listReserve(destination,
                     destination->count + employee->reports.count))
    {
        printf("Error: Not enough memory to remove this employee safely.\n");
        return;
    }

    listRemove(destination, employee);

    for (i = 0; i < employee->reports.count; i++)
    {
        Employee *report = employee->reports.items[i];

        destination->items[destination->count] = report;
        destination->count++;

        report->manager = newManager;
    }

    printf("Employee %d (%s) removed.\n",
           employee->id, employee->name);

    if (employee->reports.count > 0)
    {
        if (newManager == NULL)
        {
            printf("Their reports are now top-level employees.\n");
        }
        else
        {
            printf("Their reports now report to %d (%s).\n",
                   newManager->id, newManager->name);
        }
    }

    free(employee->reports.items);
    free(employee);
}

void showEmployeeHierarchy(void)
{
    int employeeId;
    Employee *employee;

    if (!readInteger("Enter Employee ID: ", &employeeId))
    {
        printf("Error: Invalid Employee ID.\n");
        return;
    }

    employee = findEmployee(employeeId);

    if (employee == NULL)
    {
        printf("Error: Employee ID %d not found.\n", employeeId);
        return;
    }

    printf("\n===== HIERARCHY OF EMPLOYEE %d =====\n", employeeId);

    displayHierarchy(employee, 0);

    printf("Total Employees Under %d: %d\n",
           employeeId, countReports(employee));
}

void showCompleteHierarchy(void)
{
    int i;
    int totalEmployees = 0;

    printf("\n===== ORGANIZATION HIERARCHY =====\n");

    if (topLevelEmployees.count == 0)
    {
        printf("No employees in the organization.\n");
    }

    for (i = 0; i < topLevelEmployees.count; i++)
    {
        displayHierarchy(topLevelEmployees.items[i], 0);

        totalEmployees += 1 + countReports(topLevelEmployees.items[i]);
    }

    printf("Total Employees: %d\n", totalEmployees);
}

int main(void)
{
    int choice;
    int i;

    while (1)
    {
        printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
        printf("1. Add Employee\n");
        printf("2. Remove Employee\n");
        printf("3. Display Employee Hierarchy\n");
        printf("4. Display Complete Hierarchy\n");
        printf("5. Exit\n");

        if (!readInteger("Enter your choice: ", &choice))
        {
            if (feof(stdin))
            {
                break;
            }

            printf("Invalid choice. Please enter 1-5.\n");
            continue;
        }

        switch (choice)
        {
            case 1:
                addEmployee();
                break;

            case 2:
                removeEmployee();
                break;

            case 3:
                showEmployeeHierarchy();
                break;

            case 4:
                showCompleteHierarchy();
                break;

            case 5:
                for (i = 0; i < topLevelEmployees.count; i++)
                {
                    freeEmployeeTree(topLevelEmployees.items[i]);
                }

                free(topLevelEmployees.items);

                printf("Exiting...\n");
                return 0;

            default:
                printf("Invalid choice. Please enter 1-5.\n");
        }
    }

    for (i = 0; i < topLevelEmployees.count; i++)
    {
        freeEmployeeTree(topLevelEmployees.items[i]);
    }

    free(topLevelEmployees.items);

    return 0;
}
