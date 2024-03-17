// Display Current Rules

// Implement a function that iterates over the rules array and prints out each rule. 
// his function can be called before adding a new rule or upon user request.

void display_rules() {
    if (ruleCount == 0) {
        printf("No rules have been added yet.\n");
        return;
    }
    
    printf("Current firewall rules:\n");
    for (int i = 0; i < ruleCount; i++) {
        printf("Rule %d: IP Address: %s, Port: %d, Action: %s\n", 
               i + 1, 
               rules[i].ip_address, 
               rules[i].port, 
               rules[i].allow ? "Allow" : "Deny");
    }
}


// Modify the add_rule function to first check if the rule being added already exists in the array. 
// If it does, you can either prevent the user from adding the duplicate rule or update 
// the existing rule based on the new input.

int rule_exists(const char* ip_address, int port) {
    for (int i = 0; i < ruleCount; i++) {
        if (strcmp(rules[i].ip_address, ip_address) == 0 && rules[i].port == port) {
            return i; // Return the index of the existing rule
        }
    }
    return -1; // Indicate that the rule does not exist
}

void add_or_update_rule(const char* ip_address, int port, int allow) {
    int index = rule_exists(ip_address, port);
    if (index != -1) {
        // Rule exists, update it
        rules[index].allow = allow;
        printf("Updated existing rule for IP %s and port %d to %s.\n", ip_address, port, allow ? "Allow" : "Deny");
    } else {
        // Rule does not exist, add a new one
        add_rule(ip_address, port, allow);
        printf("Added new rule for IP %s and port %d to %s.\n", ip_address, port, allow ? "Allow" : "Deny");
    }
}

// Integrating into Main Function

// Incorporate the display and check functionality into your main program flow. 
// For example, you might start the program by displaying existing rules (if any) 
// and then prompt the user for what they want to do next.

int main(int argc, char *argv[]) {
    // Example menu-driven loop
    char choice;
    do {
        printf("\nFirewall Configuration\n");
        printf("1. Display Rules\n");
        printf("2. Add/Update Rule\n");
        printf("3. Exit\n");
        printf("Enter choice: ");
        scanf(" %c", &choice);

        switch(choice) {
            case '1':
                display_rules();
                break;
            case '2':
                // Assume add_or_update_rule is called with user-supplied arguments
                break;
            case '3':
                printf("Exiting program.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != '3');
    
    // Cleanup before exiting
    free(rules);
    return 0;
}
