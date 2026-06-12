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

using strvec = std::vector<std::string>;

    strvec query_analyser(std::string prompt);
    strvec knowledge_base(const strvec keywords);
    std::string context_builder(const std::string prompt, const strvec records);
    void model_runtime(const std::string input);

    strvec split_string(const std::string &input);
    strvec stopping_words();
    strvec knowledge_base();

    int main(void) {

        while (true) {
            // Get user input
            std::string prompt;
            std::cout << "Enter Prompt >> ";
            std::getline(std::cin, prompt);

            if (prompt == "quit")
                break;

            // Process the prompt for keywords
            strvec keywords = query_analyser(prompt);

            // Retrieve records containing keywords 
            strvec records = knowledge_base(keywords);

            // Build prompt 
            std::string input = context_builder(prompt, records);

            // Send it to the model
            model_runtime(input);

        }
        return 0;
    }

    /*Strips out stopping words and returns a vector of keywords only 
     *
     * TODO: Keep context of question, embeddings
     */
    strvec query_analyser(std::string prompt) {
        std::transform(prompt.begin(), prompt.end(), prompt.begin(), 
                [](unsigned char c) { return std::tolower(c); });

        strvec stop_words = stopping_words();
        strvec prompt_vec = split_string(prompt);
        strvec keywords;

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
    strvec split_string(const std::string &input) {
        std::istringstream stream(input);
        strvec output;
        std::string word;
        while (stream >> word) 
            output.push_back(word);
        return output;
    }

    /* v1: match records from the "knowledge base" which is just hard coded strings 
     *     for now.
     *
     * v2: Interchangable database with specialised knowledge bases.
     */
    strvec knowledge_base(const strvec keywords) {
        strvec records = knowledge_base();
        strvec context;

        for (auto rec : records) {
            // Process each record, lower case them, strip stopping words
            strvec record_keywords = query_analyser(rec);

            // For each word in the keywords from the prompt 
            for (auto word : keywords) 
                // For each word in the keywords from the current record 
                for (auto recword : record_keywords) 
                    // If one word matches, then add the whole untouched record
                    if (word == recword) {
                        context.push_back(rec);
                        continue;
                    }
        }
        return context;
    }

    /* v1: Add the context from the knowledge base into a generic message scaffolding 
     *     and the original prompt
     * 
     * v2: Remove duplicate knowledge, add context around retrieved knowledge
     */
    std::string context_builder(const std::string prompt, const strvec records) {
        std::string output = "context: "; 
        for (auto rec : records)
            output += rec + " ";
        output += " question: " + prompt;
        return output;
    }

    /* Send the prompt to the model, incredible hacky*/
    void model_runtime(const std::string input) {
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
            "\"prompt\": \"" + input + "\","
            "\"stream\": true,"
            "\"think\": false}";

        std::string http_req = 
            "POST /api/generate HTTP/1.1\r\n"
            "Host: localhost:11434\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "\r\n" + 
            body;

        std::cout << input;

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

    /* Hard coded set of stopping words*/
    strvec stopping_words() {
        strvec words = {"a","about","above","after","again","against","all",
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

    /* Hard coded set of knowledge*/
    strvec knowledge_base() {
        strvec knowledge = {
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
