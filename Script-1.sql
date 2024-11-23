CREATE TABLE questions (
    id SERIAL PRIMARY KEY,         
    text TEXT NOT NULL,            
    difficulty INT NOT NULL CHECK (difficulty BETWEEN 1 AND 10), 
    correct_option INT NOT NULL    
);

CREATE TABLE answers (
    id SERIAL PRIMARY KEY,         
    question_id INT NOT NULL,      
    answer_number INT NOT NULL CHECK (answer_number BETWEEN 1 AND 4), 
    text TEXT NOT NULL,            
    FOREIGN KEY (question_id) REFERENCES questions(id) ON DELETE CASCADE
);

CREATE TABLE hints (
    id SERIAL PRIMARY KEY,         
    question_id INT NOT NULL,     
    type VARCHAR(50) NOT NULL,     
    content TEXT,                  
    FOREIGN KEY (question_id) REFERENCES questions(id) ON DELETE CASCADE
);

INSERT INTO questions (text, difficulty, correct_option)
VALUES 
    ('What is the capital of France?', 2, 1),
    ('What is 5 + 7?', 1, 3);

SELECT q.text AS question, a.answer_number, a.text AS answer
FROM questions q
JOIN answers a ON q.id = a.question_id
WHERE q.id = 1;

INSERT INTO answers (question_id, answer_number, text)
VALUES
    (1, 1, 'Paris'),
    (1, 2, 'Berlin'),
    (1, 3, 'Madrid'),
    (1, 4, 'Rome'),
    (2, 1, '10'),
    (2, 2, '11'),
    (2, 3, '12'),
    (2, 4, '13');


INSERT INTO hints (question_id, type, content)
VALUES
    (1, 'audience', 'Option 1: 70%, Option 2: 10%, Option 3: 10%, Option 4: 10%'),
    (1, '50/50', 'Remaining options: Option 1 and Option 4'),
    (2, 'friend', 'I think the answer is Option 3.');
