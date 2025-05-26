#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_LENGTH 512
#define MAX_QUESTIONS 100

typedef struct {
    char question[256];
    char answer[256];
} QA;

typedef struct {
    char name[32];
    int score;
} LeaderboardEntry;


void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

int comparator(const void *a, const void *b) {
    LeaderboardEntry *entryA = (LeaderboardEntry *)a;
    LeaderboardEntry *entryB = (LeaderboardEntry *)b;
    return entryB->score - entryA->score;
}

void leaderboardLog(const char *userName, int score) {
    LeaderboardEntry entries[100];
    int count = 0;
    FILE *fp = fopen("leaderboard.txt", "r");
    if (fp != NULL) {
        char line[64];
        while (fgets(line, sizeof(line), fp) != NULL && count < 100) {
            line[strcspn(line, "\n")] = '\0';
            char *token = strtok(line, "|");
            if (token) {
                strncpy(entries[count].name, token, sizeof(entries[count].name));
                entries[count].name[sizeof(entries[count].name)-1] = '\0';
                token = strtok(NULL, "|");
                if (token)
                    entries[count].score = atoi(token);
                else
                    entries[count].score = 0;
                count++;
            }
        }
        fclose(fp);
    }
    if (count < 100) {
        strncpy(entries[count].name, userName, sizeof(entries[count].name));
        entries[count].name[sizeof(entries[count].name)-1] = '\0';
        entries[count].score = score;
        count++;
    }
    qsort(entries, count, sizeof(LeaderboardEntry), comparator);
    
    fp = fopen("leaderboard.txt", "w");
    if (fp != NULL) {
        for (int i = 0; i < count && i < 10; i++) {
            fprintf(fp, "%s|%d\n", entries[i].name, entries[i].score);
        }
        fclose(fp);
    } else {
        perror("Error writing leaderboard file");
    }
}

void displayLeaderboard() {
    printf("\n=== Leaderboard ===\n");
    FILE *fp = fopen("leaderboard.txt", "r");
    if (fp == NULL) {
        printf("No leaderboard available.\n");
        return;
    }
    
    char line[64];
    int rank = 1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\n")] = '\0';
        char *token = strtok(line, "|");
        if (token) {
            char name[32];
            strncpy(name, token, sizeof(name));
            name[sizeof(name)-1] = '\0';
            token = strtok(NULL, "|");
            int score = 0;
            if (token)
                score = atoi(token);
            if (rank == 1) {
                printf("\033[1;33m%d. %s - %d\033[0m\n", rank, name, score);
            } else if (rank == 2) {
                printf("\033[1;37m%d. %s - %d\033[0m\n", rank, name, score);
            } else if (rank == 3) {
                printf("\033[38;5;94m%d. %s - %d\033[0m\n", rank, name, score);
            } else {
                printf("%d. %s - %d\n", rank, name, score);
            }
            rank++;
        }
    }
    fclose(fp);
    printf("===================\n");
    printf("Press Enter to return to main menu...\n");
    getchar();
    clearScreen();
}

void endScreen(int s) {
    printf("\n=== Game over! ===\n");
    printf("Score: %d\n", s);
    printf("Enter your name: ");
    char userName[32];
    fgets(userName, sizeof(userName), stdin);
    userName[strcspn(userName, "\n")] = '\0';
    leaderboardLog(userName, s);
    clearScreen();
}

void showMenu() {
    printf("\n\033[1m=== Trivia Game Menu ===\033[0m\n");
    printf("\033[1;32m1. Start Game\033[0m\n");
    printf("\033[1;33m2. Display Leaderboard\033[0m\n");
    printf("\033[1;31m3. Exit\033[0m\n");
}

void startGame() {
    int wrongAnswerCount = 0;
    int rightAnswerCount = 0;
    printf("\nStarting trivia game...\n");

    QA questions[MAX_QUESTIONS];
    int qCount = 0;
    FILE *fp = fopen("questions.txt", "r");
    if (fp == NULL) {
        perror("Error opening questions.txt");
        return;
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), fp) != NULL && qCount < MAX_QUESTIONS) {
        line[strcspn(line, "\n")] = '\0';
        char *token = strtok(line, "|");
        if (!token)
            continue;
        strncpy(questions[qCount].question, token, sizeof(questions[qCount].question));
        questions[qCount].question[sizeof(questions[qCount].question)-1] = '\0';

        token = strtok(NULL, "|");
        if (!token)
            continue;
        strncpy(questions[qCount].answer, token, sizeof(questions[qCount].answer));
        questions[qCount].answer[sizeof(questions[qCount].answer)-1] = '\0';
        qCount++;
    }
    fclose(fp);

    if (qCount == 0) {
        printf("No questions available.\n");
        return;
    }

    srand((unsigned int)time(NULL));
    for (int i = qCount - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        QA temp = questions[i];
        questions[i] = questions[j];
        questions[j] = temp;
    }

    for (int i = 0; i < qCount; i++) {
        char *qmark = strchr(questions[i].question, '?');
        if (qmark) {
            int prefixLen = (int)(qmark - questions[i].question + 1);
            printf("Question: ");
            fwrite(questions[i].question, 1, prefixLen, stdout);
            if (questions[i].question[prefixLen] != '\0') {
            printf("\033[1;33m%s\033[0m", questions[i].question + prefixLen);
            }
            printf("\n");
        } else {
            printf("Question: %s\n", questions[i].question);
        }
        printf("Enter your answer: ");
        char userAnswer[256];
        fgets(userAnswer, sizeof(userAnswer), stdin);
        userAnswer[strcspn(userAnswer, "\n")] = '\0';

        if (strcmp(userAnswer, questions[i].answer) == 0) {
            rightAnswerCount++;
            printf("Correct!\n");
        } else {
            wrongAnswerCount++;
            printf("Incorrect! The correct answer is: %s\n", questions[i].answer);
        }
        printf("-----\n");
        printf("Press Enter to continue...\n");
        getchar();
        clearScreen();

        if (wrongAnswerCount == 3) {
            endScreen(rightAnswerCount);
            return;
        }
    }
    endScreen(rightAnswerCount);
}

int main() {
    int choice;
    do {
        showMenu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();
        switch (choice) {
            case 1:
                clearScreen();
                startGame();
                break;
            case 2:
                clearScreen();
                displayLeaderboard();
                break;
            case 3:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 3);

    return 0;
}
