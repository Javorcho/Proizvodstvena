#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define MAX_QUESTIONS 100
#define MAX_LENGTH 256
#define FILENAME "questions.dat"
#define KEY_SIZE 16 
#define IV_SIZE 16 

typedef struct {
    char text[MAX_LENGTH];
    int difficulty;
    char options[4][MAX_LENGTH];
    int correctOption;
} Question;

typedef struct {
    Question questions[MAX_QUESTIONS];
    int count;
} QuestionCollection;

void encryptData(char *data, size_t length, int key) {
    for (size_t i = 0; i < length; i++) {
        if (data[i] >= 'a' && data[i] <= 'z') {
            data[i] = 'a' + (data[i] - 'a' + key) % 26;
        } else if (data[i] >= 'A' && data[i] <= 'Z') {
            data[i] = 'A' + (data[i] - 'A' + key) % 26;
        }
    }
}

void decryptData(char *data, size_t length, int key) {
    for (size_t i = 0; i < length; i++) {
        if (data[i] >= 'a' && data[i] <= 'z') {
            data[i] = 'a' + (data[i] - 'a' - key + 26) % 26;
        } else if (data[i] >= 'A' && data[i] <= 'Z') {
            data[i] = 'A' + (data[i] - 'A' - key + 26) % 26;
        }
    }
}

void saveQuestionsToFile(QuestionCollection *collection, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Cannot open file for writing");
        return;
    }

    size_t dataSize = sizeof(QuestionCollection);
    char *data = malloc(dataSize);
    memcpy(data, collection, dataSize);

    encryptData(data, dataSize, 3);

    fwrite(data, 1, dataSize, file);
    free(data);
    fclose(file);
}

void loadQuestionsFromFile(QuestionCollection *collection, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Cannot open file for reading");
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *data = malloc(fileSize);
    fread(data, 1, fileSize, file);

    decryptData(data, fileSize, 3);
    memcpy(collection, data, sizeof(QuestionCollection));

    free(data);
    fclose(file);
}

void addQuestion(QuestionCollection *collection) {
    if (collection->count >= MAX_QUESTIONS) {
        printf("Maximum number of questions reached.\n");
        return;
    }

    Question q;
    printf("Enter question text: ");
    fgets(q.text, MAX_LENGTH, stdin);
    q.text[strcspn(q.text, "\n")] = 0;

    printf("Enter difficulty (1-10): ");
    scanf("%d", &q.difficulty);
    getchar();

    for (int i = 0; i < 4; i++) {
        printf("Enter option %d: ", i + 1);
        fgets(q.options[i], MAX_LENGTH, stdin);
        q.options[i][strcspn(q.options[i], "\n")] = 0;
    }

    printf("Enter correct option number (1-4): ");
    scanf("%d", &q.correctOption);
    getchar();

    collection->questions[collection->count++] = q;
    saveQuestionsToFile(collection, FILENAME);
}

void editQuestion(QuestionCollection *collection) {
    int index;
    printf("Enter question number to edit (1-%d): ", collection->count);
    scanf("%d", &index);
    getchar();

    if (index < 1 || index > collection->count) {
        printf("Invalid question number.\n");
        return;
    }

    Question *q = &collection->questions[index - 1];

    printf("Editing question: %s\n", q->text);
    printf("Enter new question text (leave empty to keep current): ");
    char buffer[MAX_LENGTH];
    fgets(buffer, MAX_LENGTH, stdin);
    if (strlen(buffer) > 1) {
        buffer[strcspn(buffer, "\n")] = 0;
        strcpy(q->text, buffer);
    }

    printf("Enter new difficulty (1-10) (current: %d): ", q->difficulty);
    int newDifficulty;
    scanf("%d", &newDifficulty);
    getchar();
    if (newDifficulty >= 1 && newDifficulty <= 10) {
        q->difficulty = newDifficulty;
    }

    for (int i = 0; i < 4; i++) {
        printf("Enter new option %d (leave empty to keep current): ", i + 1);
        fgets(buffer, MAX_LENGTH, stdin);
        if (strlen(buffer) > 1) {
            buffer[strcspn(buffer, "\n")] = 0;
            strcpy(q->options[i], buffer);
        }
    }

    printf("Enter new correct option number (1-4) (current: %d): ", q->correctOption);
    int newCorrectOption;
    scanf("%d", &newCorrectOption);
    getchar();
    if (newCorrectOption >= 1 && newCorrectOption <= 4) {
        q->correctOption = newCorrectOption;
    }

    saveQuestionsToFile(collection, FILENAME);
}

void printMenu() {
    printf("\nMenu:\n");
    printf("1. Start Game\n");
    printf("2. Add Question\n");
    printf("3. Edit Question\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
}

void audienceHelp(Question *q) {
    int votes[4] = {0};
    int totalVotes = 100;

    for (int i = 0; i < 4; i++) {
        if (i + 1 == q->correctOption) {
            votes[i] = (q->difficulty <= 3) ? 80 : (q->difficulty <= 6) ? 60 : 30;
        } else {
            votes[i] = (totalVotes - votes[q->correctOption - 1]) / 3;
        }
    }

    printf("Audience votes:\n");
    for (int i = 0; i < 4; i++) {
        printf("Option %d: %d%%\n", i + 1, votes[i]);
    }
}

void phoneAFriend(Question *q) {
    int correctChance = (q->difficulty <= 3) ? 80 : (q->difficulty <= 6) ? 60 : 30;
    int friendAnswer = (rand() % 100 < correctChance) ? q->correctOption : (rand() % 4) + 1;
    printf("Your friend thinks the answer is: %d\n", friendAnswer);
}

void startGame(QuestionCollection *collection) {
    if (collection->count < 2) {
        printf("Not enough questions to start the game.\n");
        return;
    }

    srand(time(NULL));
    int usedQuestions[MAX_QUESTIONS] = {0};
    Question selectedQuestions[10];

    for (int i = 0; i < 10; i++) {
        int index;
        do {
            index = rand() % collection->count;
        } while (usedQuestions[index]);
        usedQuestions[index] = 1;
        selectedQuestions[i] = collection->questions[index];
    }

    int joker5050 = 1, jokerFriend = 1, jokerAudience = 1;

    for (int i = 0; i < 10; i++) {
        Question *q = &selectedQuestions[i];
        int reducedOptions[2] = {0};
        int jokerUsedInThisQuestion = 0;

        while (1) {
            printf("Question %d: %s\n", i + 1, q->text);

            if (joker5050 == 0 && jokerUsedInThisQuestion == 5) {
                printf("%d. %s\n", reducedOptions[0], q->options[reducedOptions[0] - 1]);
                printf("%d. %s\n", reducedOptions[1], q->options[reducedOptions[1] - 1]);
            } else {
                for (int j = 0; j < 4; j++) {
                    printf("%d. %s\n", j + 1, q->options[j]);
                }
            }

            if (joker5050 || jokerFriend || jokerAudience) {
                printf("Available jokers: ");
                if (joker5050) printf("1. 50/50 ");
                if (jokerFriend) printf("2. Phone a Friend ");
                if (jokerAudience) printf("3. Audience Help ");
                printf("\n");
            }

            int answer, joker = 0;
            printf("Enter your answer (1-4) or joker number (5-7): ");
            scanf("%d", &answer);
            getchar();

            if (answer >= 5 && answer <= 7) {
                joker = answer;
                answer = 0;
            }

            if (joker == 5) {
                if (joker5050 && jokerUsedInThisQuestion == 0) {
                    joker5050 = 0;
                    jokerUsedInThisQuestion = 5;
                    int wrongAnswers[3], count = 0;
                    for (int j = 0; j < 4; j++) {
                        if (j + 1 != q->correctOption) {
                            wrongAnswers[count++] = j + 1;
                        }
                    }
                    reducedOptions[0] = q->correctOption;
                    reducedOptions[1] = wrongAnswers[rand() % 3];
                    printf("50/50 joker used. Remaining options: %d, %d\n", reducedOptions[0], reducedOptions[1]);
                } else {
                    printf("50/50 joker is not available or already used for this question.\n");
                }
            } else if (joker == 6) {
                if (jokerFriend && jokerUsedInThisQuestion == 0) {
                    jokerFriend = 0;
                    jokerUsedInThisQuestion = 6;
                    phoneAFriend(q);
                } else {
                    printf("Phone a Friend joker is not available or already used for this question.\n");
                }
            } else if (joker == 7) {
                if (jokerAudience && jokerUsedInThisQuestion == 0) {
                    jokerAudience = 0;
                    jokerUsedInThisQuestion = 7;
                    audienceHelp(q);
                } else {
                    printf("Audience Help joker is not available or already used for this question.\n");
                }
            } else if (answer >= 1 && answer <= 4) {
                int selectedAnswer = (joker5050 == 0 && jokerUsedInThisQuestion == 5) ? reducedOptions[answer - 1] : answer;
                if (selectedAnswer == q->correctOption) {
                    printf("Correct!\n");
                    break;
                } else {
                    printf("Wrong! The correct answer was: %d.\n", q->correctOption);
                    break;
                }
            } else {
                printf("Invalid input. Please try again.\n");
            }
        }
    }

    printf("Game Over. You completed the game.\n");
}

int main() {
    QuestionCollection collection = { .count = 0 };
    loadQuestionsFromFile(&collection, FILENAME);

    while (1) {
        printMenu();
        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                startGame(&collection);
                break;
            case 2:
                addQuestion(&collection);
                break;
            case 3:
                editQuestion(&collection);
                break;
            case 4:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}
