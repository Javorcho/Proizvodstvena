#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_QUESTIONS 100
#define MAX_LENGTH 256
#define FILENAME "questions.dat"

// Структура за въпрос
typedef struct {
    char text[MAX_LENGTH];
    int difficulty;
    char options[4][MAX_LENGTH];
    int correctOption;
} Question;

// Структура за колекция от въпроси
typedef struct {
    Question questions[MAX_QUESTIONS];
    int count;
} QuestionCollection;

// Функции за четене и записване на въпроси във файл
void saveQuestionsToFile(QuestionCollection *collection, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Cannot open file for writing");
        return;
    }
    fwrite(collection, sizeof(QuestionCollection), 1, file);
    fclose(file);
}

void loadQuestionsFromFile(QuestionCollection *collection, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Cannot open file for reading");
        return;
    }
    fread(collection, sizeof(QuestionCollection), 1, file);
    fclose(file);
}

// Функция за добавяне на нов въпрос
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
    getchar(); // Consume newline character

    for (int i = 0; i < 4; i++) {
        printf("Enter option %d: ", i + 1);
        fgets(q.options[i], MAX_LENGTH, stdin);
        q.options[i][strcspn(q.options[i], "\n")] = 0;
    }

    printf("Enter correct option number (1-4): ");
    scanf("%d", &q.correctOption);
    getchar(); // Consume newline character

    collection->questions[collection->count++] = q;
    saveQuestionsToFile(collection, FILENAME);
}

// Функция за редактиране на съществуващ въпрос
void editQuestion(QuestionCollection *collection) {
    int index;
    printf("Enter question number to edit (1-%d): ", collection->count);
    scanf("%d", &index);
    getchar(); // Consume newline character

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
    getchar(); // Consume newline character
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
    getchar(); // Consume newline character
    if (newCorrectOption >= 1 && newCorrectOption <= 4) {
        q->correctOption = newCorrectOption;
    }

    saveQuestionsToFile(collection, FILENAME);
}

// Функция за извеждане на менюто
void printMenu() {
    printf("\nMenu:\n");
    printf("1. Start Game\n");
    printf("2. Add Question\n");
    printf("3. Edit Question\n");
    printf("4. Exit\n");
    printf("Enter your choice: ");
}

// Функция за симулиране на помощ от публиката
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

// Функция за симулиране на обаждане на приятел
void phoneAFriend(Question *q) {
    int correctChance = (q->difficulty <= 3) ? 80 : (q->difficulty <= 6) ? 60 : 30;
    int friendAnswer = (rand() % 100 < correctChance) ? q->correctOption : (rand() % 4) + 1;
    printf("Your friend thinks the answer is: %d\n", friendAnswer);
}

// Функция за стартиране на играта
void startGame(QuestionCollection *collection) {
    if (collection->count < 10) {
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
        printf("Question %d: %s\n", i + 1, q->text);
        for (int j = 0; j < 4; j++) {
            printf("%d. %s\n", j + 1, q->options[j]);
        }

        if (joker5050 || jokerFriend || jokerAudience) {
            printf("Available jokers: ");
            if (joker5050) printf("1. 50/50 ");
            if (jokerFriend) printf("2. Phone a Friend ");
            if (jokerAudience) printf("3. Audience Help ");
            printf("\n");
        }

        int answer, joker;
        printf("Enter your answer (1-4) or joker number (5-7): ");
        scanf("%d", &answer);
        getchar(); // Consume newline character

        if (answer >= 5 && answer <= 7) {
            joker = answer;
            answer = 0;
        } else {
            joker = 0;
        }

        if (joker == 5 && joker5050) {
            joker5050 = 0;
            int wrongAnswers[3], count = 0;
            for (int j = 0; j < 4; j++) {
                if (j + 1 != q->correctOption) {
                    wrongAnswers[count++] = j + 1;
                }
            }
            printf("50/50 joker used. Remaining options: %d, %d\n", q->correctOption, wrongAnswers[rand() % 3]);
        } else if (joker == 6 && jokerFriend) {
            jokerFriend = 0;
            phoneAFriend(q);
        } else if (joker == 7 && jokerAudience) {
            jokerAudience = 0;
            audienceHelp(q);
        } else if (answer == q->correctOption) {
            printf("Correct!\n");
        } else {
            printf("Wrong! The correct answer was: %d. Game over.\n", q->correctOption);
            return;
        }
    }

    printf("Congratulations! You answered all questions correctly.\n");
}

int main() {
    QuestionCollection collection = { .count = 0 };
    loadQuestionsFromFile(&collection, FILENAME);

    while (1) {
        printMenu();
        int choice;
        scanf("%d", &choice);
        getchar(); // Consume newline character

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
