#include <stdio.h>
#include <string.h>

const int STANDARD_PRICE = 150;
const int PREMIUM_PRICE = 250;
const int RECLINER_PRICE = 400;

void getcustdetail(char name[], int *tickets, int *choice) {
    *tickets = 0;
    *choice = 0;

    printf("Enter customer name: ");
    scanf("%49s", name);

 printf("Enter number of tickets: ");
    if (scanf("%d", tickets) != 1 || *tickets < 1 || *tickets > 20) {
        printf("Error: Ticket count must be between 1 and 20.\n");
        *tickets = 0;
        return;
    }
    printf("Enter ticket category (1-Standard, 2-Premium, 3-Recliner): ");
    if (scanf("%d", choice) != 1) {
        *choice = 0;
    }
}

int getprice(int choice, char category[]) {
    switch (choice) {
        case 1:
            strcpy(category, "Standard");
            return STANDARD_PRICE;

        case 2:
            strcpy(category, "Premium");
            return PREMIUM_PRICE;

        case 3:
            strcpy(category, "Recliner");
            return RECLINER_PRICE;

        default:
            return 0;
    }
}

float calsubtotal(int ticketPrice, int tickets) {
    return (float)ticketPrice * tickets;
}

float caldiscount(float subtotal, int tickets) {
    if (tickets >= 5) {
        return subtotal * 0.10f;
    } else {
        return 0;
    }
}

void displaybill(char name[], char category[], int tickets,
                 float subtotal, float discount, float finalBill) {
      printf("\nCustomer: %s\n", name);
        printf("Ticket Category: %s\n", category);
     printf("Tickets: %d\n", tickets);
     printf("Subtotal: %.2f\n", subtotal);
    printf("Discount: %.2f\n", discount);
     printf("Final Bill: %.2f\n", finalBill);
} 

int main() {
    char name[50];
    char category[20];
    int tickets, choice;
    int ticketPrice;
    float subtotal, discount, finalBill;

    getcustdetail(name, &tickets, &choice);

    if (tickets == 0) {
        return 0;
    }

    ticketPrice = getprice(choice, category);

    if (ticketPrice == 0) {
        printf("Error: Invalid ticket category.\n");
        return 0;
    }

    subtotal = calsubtotal(ticketPrice, tickets);
    discount = caldiscount(subtotal, tickets);
    finalBill = subtotal - discount;

    displaybill(name, category, tickets, subtotal, discount, finalBill);

    return 0;
}