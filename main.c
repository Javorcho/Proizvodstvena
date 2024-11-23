#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

#define MAX_LENGTH 256

typedef struct {
    char text[MAX_LENGTH];
    int difficulty;
    char options[4][MAX_LENGTH];
    int correctOption;
} Question;

PGconn *connectDB() {
    PGconn *conn = PQconnectdb("host=localhost dbname=quiz user=user password=password");
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        exit(1);
    }
    return conn;
}

void addQuestion(PGconn *conn) {
    char questionText[MAX_LENGTH];
    int difficulty;
    char options[4][MAX_LENGTH];
    int correctOption;

    printf("Enter question text: ");
    fgets(questionText, MAX_LENGTH, stdin);
    questionText[strcspn(questionText, "\n")] = 0;

    printf("Enter difficulty (1-10): ");
    scanf("%d", &difficulty);
    getchar();

    for (int i = 0; i < 4; i++) {
        printf("Enter option %d: ", i + 1);
        fgets(options[i], MAX_LENGTH, stdin);
        options[i][strcspn(options[i], "\n")] = 0;
    }

    printf("Enter correct option number (1-4): ");
    scanf("%d", &correctOption);
    getchar();

    char query[MAX_LENGTH * 6];
    snprintf(query, sizeof(query), 
        "INSERT INTO questions (text, difficulty, correct_option) VALUES ('%s', %d, %d) RETURNING id;", 
        questionText, difficulty, correctOption);

    PGresult *res = PQexec(conn, query);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Insert question failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    int questionId = atoi(PQgetvalue(res, 0, 0));
    PQclear(res);

    for (int i = 0; i < 4; i++) {
        snprintf(query, sizeof(query),
            "INSERT INTO answers (question_id, answer_number, text) VALUES (%d, %d, '%s');",
            questionId, i + 1, options[i]);
        res = PQexec(conn, query);

        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            fprintf(stderr, "Insert answer failed: %s\n", PQerrorMessage(conn));
        }
        PQclear(res);
    }

    printf("Question added successfully!\n");
}

void listQuestions(PGconn *conn) {
    PGresult *res = PQexec(conn, 
        "SELECT q.id, q.text, q.difficulty, q.correct_option, a.answer_number, a.text AS option_text "
        "FROM questions q JOIN answers a ON q.id = a.question_id "
        "ORDER BY q.id, a.answer_number;");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Fetch questions failed: %s\n", PQerrorMessage(conn));
        PQclear(res);
        return;
    }

    int rows = PQntuples(res);
    int currentQuestion = -1;

    for (int i = 0; i < rows; i++) {
        int questionId = atoi(PQgetvalue(res, i, 0));
        if (questionId != currentQuestion) {
            if (currentQuestion != -1) printf("\n");
            currentQuestion = questionId;
            printf("Question ID: %d\n", questionId);
            printf("Text: %s\n", PQgetvalue(res, i, 1));
            printf("Difficulty: %s\n", PQgetvalue(res, i, 2));
        }
        printf("Option %s: %s\n", PQgetvalue(res, i, 4), PQgetvalue(res, i, 5));
    }

    PQclear(res);
}

void printMenu() {
    printf("\nMenu:\n");
    printf("1. Add Question\n");
    printf("2. List Questions\n");
    printf("3. Exit\n");
    printf("Enter your choice: ");
}

int main() {
    PGconn *conn = connectDB();

    while (1) {
        printMenu();
        int choice;
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                addQuestion(conn);
                break;
            case 2:
                listQuestions(conn);
                break;
            case 3:
                printf("Exiting...\n");
                PQfinish(conn);
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}
