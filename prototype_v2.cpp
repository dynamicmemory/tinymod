#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

struct Query {
    std::string prompt;
    std::vector<std::string> keywords;
};

class QueryAnalyser {
private:
public: 
    std::vector<std::string> analyse(std::string prompt) {
        std::transform(prompt.begin(), prompt.end(), prompt.begin(), 
                [](unsigned char c) { return std::tolower(c); });

        std::vector<std::string> stop_words = stopping_words();
        std::vector<std::string> prompt_vec = split_string(prompt);
        std::vector<std::string> keywords;

        for (auto pword : prompt_vec) {
            bool in = false;

            for (auto sword : stop_words)
                if (pword == sword) 
                    in = true;

            if (in) {
                in = false;
                continue;
            }
            keywords.push_back(pword);
        }

        return keywords;    
    }

    /* Utility function for splitting strings into string vectors*/
    std::vector<std::string> split_string(const std::string &input) {
        std::istringstream stream(input);
        std::vector<std::string> output;
        std::string word;
        while (stream >> word) 
            output.push_back(word);
        return output;
    }

    /* Hard coded set of stopping words*/
    std::vector<std::string> stopping_words() {
        std::vector<std::string> words = {"a","about","above","after","again","against","all",
            "am","an","and","any","are","as","at","be","because","been","before",
            "being","below","between","both","but","by","could","did","do","does",
            "doing","down","during","each","few","for","from","further","had","has",
            "have","having","he","her","here","hers","herself","him","himself","his",
            "how","i","if","in","into","is","it","its","itself","just","me","more",
            "most","my","myself","no","nor","not","now","of","off","on","once","only",
            "or","other","our","ours","ourselves","out","over","own","same","she","should",
            "so","some","such","than","that","the","their","theirs","them","themselves",
            "then","there","these","they","this","those","through","to","too","under",
            "until","up","very","was","we","were","what","when","where","which","while",
            "who","whom","why","will","with","you","your","yours","yourself","yourselves"};
        return words;
    }
};


class KnowledgeBase {
private:
public:
    std::vector<std::string> retrieve(std::vector<std::string> keywords) {
        std::vector<std::string> records = knowledge_base();
        std::vector<std::string> context;

        for (auto rec : records) {
            // Process each record, lower case them, strip stopping words
            std::transform(rec.begin(), rec.end(), rec.begin(), 
                [](unsigned char c) { return std::tolower(c); });
            std::vector<std::string> record_keywords = split_string(rec);

            // TODO Will add a record multiple times if multiple keywords are in prompt or record
            // For each word in the keywords from the prompt 
            for (auto word : keywords) { 
                // For each word in the keywords from the current record 
                for (auto recword : record_keywords) 
                    // If one word matches, then add the whole untouched record
                    if (word == recword) {
                        context.push_back(rec);
                        continue;
                    }
            }
        }
        return context;
    }

    /* Utility function for splitting strings into string vectors*/
    std::vector<std::string> split_string(const std::string &input) {
        std::istringstream stream(input);
        std::vector<std::string> output;
        std::string word;
        while (stream >> word) 
            output.push_back(word);
        return output;
    }

    /* Hard coded set of knowledge*/
    std::vector<std::string> knowledge_base() {
        std::vector<std::string> knowledge = {
            "The user is interested in cybernetics and control theory",
            "The user has studied Ashby's Law of Requisite Variety",
            "The user believes regulation is a fundamental concept in intelligence",
            "The user proposed a higher-level regulator that creates lower-level regulators",
            "The user is interested in building AI systems from cybernetic principles rather than scaling alone",
            "The user believes intelligence may emerge from layered regulatory structures",
            "The user is studying software development and artificial intelligence",
            "The user prefers Linux as a primary operating system",
            "The user primarily programs in C",
            "The user also uses C++ and Python",
            "The user wants to build local AI systems rather than relying entirely on cloud services",
            "The user is building a local RAG system as a portfolio project",
        };
        return knowledge;
    }
};


class ContextBuilder {
private: 
public:
    std::string build(std::string prompt, std::vector<std::string> context) {
        std::string output = "context: "; 
        for (auto con : context)
            output += con + " ";
        output += " question: " + prompt;
        return output;
    }
};


class ModelRuntime {
private:
public:
    void query(std::string prompt) {

        const char host[] = "127.0.0.1";
        const char port[] = "11434";

        struct addrinfo hints, *addr;
        ::memset(&hints, 0, sizeof(addrinfo));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_family = AF_UNSPEC;

        int status = ::getaddrinfo(host, port, &hints, &addr);
        if (status != 0) {
            std::cout << "getaddrinfo failed\n";
            return;
        }

        int server = ::socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (server < 0) {
            std::cout << "socket failed\n";
            return;
        }

        int connect = ::connect(server, addr->ai_addr, addr->ai_addrlen);
        if (connect < 0) {
            std::cout << "connect failed\n";
            return;
        }
        ::freeaddrinfo(addr);

        std::string body = 
            // "{\"model\": \"llama3.1:8B\","
            "{\"model\": \"qwen3:8B\","
            "\"prompt\": \"" + prompt + "\","
            "\"stream\": true,"
            "\"think\": false}";

        std::string http_req = 
            "POST /api/generate HTTP/1.1\r\n"
            "Host: localhost:11434\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "\r\n" + 
            body;

        std::cout << prompt;

        ssize_t send = ::send(server, http_req.c_str(), http_req.size(), 0);
        std::cout << "Size sent: " << send << "\n";

        while (1) {

            // std::cout << "in the while loop\n";
            char res[4096];
            char *p;
            int recieve = ::recv(server, res, sizeof(res), 0);

            // std::cout << res ;
            if (recieve <=0) {
                std::cout << "Connection closed or stalled \n";
                break;
            }

            // Find the response and print it to the terminal char by char
            if ((p=strstr(res, "\"response\":\""))) {
                p+=12;

                while (*p && *p != '"')
                    putchar(*p++);
            }

            fflush(stdout);
            // Exit loop on response finished
            if ((p=strstr(res, "\"done\":true"))) {
                printf("\n");
                break;
            }
        }
    }
};


class Agent {
private:
public:
    QueryAnalyser analyser;
    KnowledgeBase kb;
    ContextBuilder builder; 
    ModelRuntime model;

    void ask(std::string prompt) {
        auto keywords = analyser.analyse(prompt);
        auto context = kb.retrieve(keywords);
        auto reconstructed = builder.build(prompt, context);
        model.query(reconstructed);
    }
};

int main(void) {

    Agent smith = Agent();

    while (true) {
        // Get user input
        std::string prompt; 
        std::cout << "Enter Prompt >> ";
        std::getline(std::cin, prompt);

        if (prompt == "quit")
            break;

        smith.ask(prompt);
    }
    return 0;
}
