
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Players_data.h"

typedef struct Cricketer {
    int id;
    char name[51];
    char team[51];
    int role;
    int runs;
    float avg, sr;
    int wkts;
    float er, pi;
    struct Cricketer *next, *rnext;
} Cricketer;

typedef struct {
    int id;
    char name[51];
    int total;
    float srSum;
    int srCnt;
    Cricketer *head, *tail, *rhead[4];
} Team;

Team T[10];

const char *order[10] = {
    "India", "Australia", "England", "Pakistan", "New Zealand",
    "South Africa", "Bangladesh", "Sri Lanka", "Afghanistan", "West Indies"
};

const char *roleName[] = {"", "Batsman", "Bowler", "All-rounder"};

float calcpi(Cricketer *c) {
    if (c->role == 1) return c->avg * c->sr / 100;
    if (c->role == 2) return c->wkts * 2 + (100 - c->er);
    return c->avg * c->sr / 100 + c->wkts * 2;
}

float avgsr(Team *t) {
    return t->srCnt ? t->srSum / t->srCnt : 0;
}

void addtoteam(Team *t, Cricketer *c) {
    c->pi = calcpi(c);
    c->next = NULL;

    if (t->tail) t->tail->next = c;
    else t->head = c;

    t->tail = c;

    Cricketer **p = &t->rhead[c->role];

    while (*p && (*p)->pi > c->pi)
        p = &(*p)->rnext;

    c->rnext = *p;
    *p = c;

    t->total++;

    if (c->role != 2) {
        t->srSum += c->sr;
        t->srCnt++;
    }
}

Team *findteam(int id) {
    int lo = 0, hi = 9;

    while (lo <= hi) {
        int m = (lo + hi) / 2;

        if (T[m].id == id) return &T[m];

        if (T[m].id < id) lo = m + 1;
        else hi = m - 1;
    }

    return NULL;
}

int roleof(const char *s) {
    if (strcmp(s, "Batsman") == 0) return 1;
    if (strcmp(s, "Bowler") == 0) return 2;
    return 3;
}

void initdata() {
    for (int i = 0; i < 10; i++) {
        T[i].id = i + 1;
        strcpy(T[i].name, order[i]);
    }

    for (int i = 0; i < playerCount; i++) {
        for (int j = 0; j < 10; j++) {
            if (strcmp(players[i].team, order[j]) != 0) continue;

            Cricketer *c = calloc(1, sizeof(Cricketer));

            c->id = players[i].id;
            strcpy(c->name, players[i].name);
            strcpy(c->team, players[i].team);

            c->role = roleof(players[i].role);
            c->runs = players[i].totalRuns;
            c->avg = players[i].battingAverage;
            c->sr = players[i].strikeRate;
            c->wkts = players[i].wickets;
            c->er = players[i].economyRate;

            addtoteam(&T[j], c);
            break;
        }
    }
}

void printheader() {
    printf("%-5s %-24s %-14s %-12s %6s %6s %6s %5s %5s %9s\n",
           "ID", "Name", "Team", "Role", "Runs",
           "Avg", "SR", "Wkts", "ER", "PerfIdx");
}

void printrow(Cricketer *c) {
    printf("%-5d %-24s %-14s %-12s %6d %6.1f %6.1f %5d %5.1f %9.2f\n",
           c->id, c->name, c->team, roleName[c->role],
           c->runs, c->avg, c->sr, c->wkts, c->er, c->pi);
}

void addplayer() {
    int tid;

    printf("Enter Team ID to add player: ");
    scanf("%d", &tid);

    Team *t = findteam(tid);

    if (!t) {
        printf("Team not found.\n");
        return;
    }

    Cricketer *c = calloc(1, sizeof(Cricketer));

    printf("Player ID: ");
    scanf("%d", &c->id);

    printf("Name: ");
    scanf(" %50[^\n]", c->name);

    printf("Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    scanf("%d", &c->role);

    if (c->role < 1 || c->role > 3) {
        printf("Invalid role.\n");
        free(c);
        return;
    }

    printf("Total Runs: ");
    scanf("%d", &c->runs);

    printf("Batting Average: ");
    scanf("%f", &c->avg);

    printf("Strike Rate: ");
    scanf("%f", &c->sr);

    printf("Wickets: ");
    scanf("%d", &c->wkts);

    printf("Economy Rate: ");
    scanf("%f", &c->er);

    strcpy(c->team, t->name);

    addtoteam(t, c);

    printf("Player added successfully to Team %s!\n", t->name);
}

void showteam() {
    int tid;

    printf("Enter Team ID: ");
    scanf("%d", &tid);

    Team *t = findteam(tid);

    if (!t) {
        printf("Team not found.\n");
        return;
    }

    printf("Players of Team %s:\n", t->name);

    printheader();

    for (Cricketer *c = t->head; c; c = c->next)
        printrow(c);

    printf("Total Players: %d\n", t->total);
    printf("Average Batting Strike Rate: %.2f\n", avgsr(t));
}

int cmpteams(const void *a, const void *b) {
    float x = avgsr(*(Team **)a);
    float y = avgsr(*(Team **)b);

    return (y > x) - (y < x);
}

void showteamsbysr() {
    Team *arr[10];

    for (int i = 0; i < 10; i++)
        arr[i] = &T[i];

    qsort(arr, 10, sizeof(Team *), cmpteams);

    printf("%-4s %-15s %-12s %s\n",
           "ID", "Team Name", "Avg Bat SR", "Total Players");

    for (int i = 0; i < 10; i++)
        printf("%-4d %-15s %-12.2f %d\n",
               arr[i]->id, arr[i]->name,
               avgsr(arr[i]), arr[i]->total);
}

void topk() {
    int tid, role, k;

    printf("Enter Team ID: ");
    scanf("%d", &tid);

    Team *t = findteam(tid);

    if (!t) {
        printf("Team not found.\n");
        return;
    }

    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    scanf("%d", &role);

    if (role < 1 || role > 3) {
        printf("Invalid role.\n");
        return;
    }

    printf("Enter number of players: ");
    scanf("%d", &k);

    printf("Top %d %ss of Team %s:\n", k, roleName[role], t->name);

    printheader();

    Cricketer *c = t->rhead[role];

    for (int i = 0; i < k && c; i++, c = c->rnext)
        printrow(c);
}

void merge(Cricketer **src, Cricketer **dst, int lo, int mid, int hi) {
    int i = lo, j = mid, k = lo;

    while (i < mid && j < hi)
        dst[k++] = src[i]->pi >= src[j]->pi ? src[i++] : src[j++];

    while (i < mid) dst[k++] = src[i++];
    while (j < hi) dst[k++] = src[j++];
}

void allbyrole() {
    int role;

    printf("Enter Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    scanf("%d", &role);

    if (role < 1 || role > 3) {
        printf("Invalid role.\n");
        return;
    }

    int n = 0;

    for (int i = 0; i < 10; i++)
        for (Cricketer *c = T[i].rhead[role]; c; c = c->rnext)
            n++;

    if (n == 0) {
        printf("No players found.\n");
        return;
    }

    Cricketer **a = malloc(n * sizeof(Cricketer *));
    Cricketer **b = malloc(n * sizeof(Cricketer *));

    int bound[11], runs = 10, idx = 0;

    for (int i = 0; i < 10; i++) {
        bound[i] = idx;

        for (Cricketer *c = T[i].rhead[role]; c; c = c->rnext)
            a[idx++] = c;
    }

    bound[10] = n;

    while (runs > 1) {
        int nb[11], nr = 0;

        for (int i = 0; i < runs; i += 2) {
            nb[nr++] = bound[i];

            if (i + 1 < runs)
                merge(a, b, bound[i], bound[i + 1], bound[i + 2]);
            else
                memcpy(b + bound[i], a + bound[i],
                       (bound[i + 1] - bound[i]) * sizeof(Cricketer *));
        }

        nb[nr] = n;

        memcpy(bound, nb, (nr + 1) * sizeof(int));

        runs = nr;

        Cricketer **tmp = a;
        a = b;
        b = tmp;
    }

    printf("%ss of all teams:\n", roleName[role]);

    printheader();

    for (int i = 0; i < n; i++)
        printrow(a[i]);

    free(a);
    free(b);
}

int main() {
    initdata();

    int ch;

    do {
        printf("\n==============================================================\n");
        printf("ICC ODI Player Performance Analyzer\n");
        printf("==============================================================\n");

        printf("1. Add Player to Team\n");
        printf("2. Display Players of a Specific Team\n");
        printf("3. Display Teams by Average Batting Strike Rate\n");
        printf("4. Display Top K Players of a Specific Team by Role\n");
        printf("5. Display all Players of specific role Across All Teams by performance index\n");
        printf("6. Exit\n");

        printf("Enter your choice: ");

        if (scanf("%d", &ch) != 1) break;

        switch (ch) {
            case 1: addplayer(); break;
            case 2: showteam(); break;
            case 3: showteamsbysr(); break;
            case 4: topk(); break;
            case 5: allbyrole(); break;
            case 6: break;
            default: printf("Invalid choice.\n");
        }

    } while (ch != 6);

    return 0;
}
