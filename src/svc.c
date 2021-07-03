/**
 * @file svc.c
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */
#include "svc.h"

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include "config.h"
#include "console.h"
#include "process.h"
#include "signals.h"
#include "tty.h"
#include "utilities.h"

const char* AVAILABLE_DIR = "/etc/quickinit/services/available";
const char* ENABLED_DIR = "/etc/quickinit/services/enabled";

struct service_node* service_head = NULL;  // head of the list

/**
 * @brief Adds service to the list
 * 
 * @param head head of the list
 * @param priority_start start priority of the service
 * @param priority_stop stop priority of the service
 * @param name name of the service
 */
static void addToList(struct service_node** head, uint16_t priority_start, uint16_t priority_stop, const char* name) {
    struct service_node* new_service = (struct service_node*)malloc(sizeof(struct service_node));

    new_service->pid = 0;
    new_service->time = 0;
    new_service->state = SERVICE_STOPPED;
    new_service->priority_start = priority_start;
    new_service->priority_stop = priority_stop;
    strcpy(new_service->name, name);

    new_service->next = *head;
    *head = new_service;
}

static struct service_node* list_prevNode(struct service_node* a) {
    if (service_head == a) {
        return NULL;
    }
    struct service_node* temp = service_head;  // temp is current node
    struct service_node* prev_a = NULL;

    while (temp && temp != a) {  // search until find null or a node
        prev_a = temp;           // find previous node
        temp = temp->next;
    }
    return prev_a;
}

/**
 * @brief Swaps two services at the list
 * 
 * @param a pointer to service node
 * @param b pointer to service node
 */
void service_swapServices(struct service_node** a, struct service_node** b) {
    if ((service_head) == NULL || (*a) == NULL || (*b) == NULL)
        return;

    // find previous nodes
    struct service_node* prev_a = list_prevNode(*a);
    struct service_node* prev_b = list_prevNode(*b);

    if (prev_a) prev_a->next = (*b);  // prev a is prev b
    if (prev_b) prev_b->next = (*a);  // prev b is prev a

    // swap next fields
    struct service_node* temp = NULL;
    temp = (*a)->next;
    (*a)->next = (*b)->next;
    (*b)->next = temp;

    //if a or b was head, swap head
    if ((service_head) == (*a))
        service_head = *b;
    else if ((service_head) == (*b))
        service_head = *a;
}

/**
 * @brief Sorts services ascending by start priority
 * 
 */
static void sortServicesAscendingByStartPriority() {
    for (struct service_node* temp1 = service_head; temp1 != NULL; temp1 = temp1->next) {
        for (struct service_node* temp2 = temp1->next; temp2 != NULL; temp2 = temp2->next) {
            if (temp2->priority_start < temp1->priority_start) {
                service_swapServices(&temp1, &temp2);
            }
        }
    }
}

/**
 * @brief Sorts services ascending by stop priority
 * 
 */
static void sortServicesAscendingByStopPriority() {
    for (struct service_node* temp1 = service_head; temp1 != NULL; temp1 = temp1->next) {
        for (struct service_node* temp2 = temp1->next; temp2 != NULL; temp2 = temp2->next) {
            if (temp2->priority_stop < temp1->priority_stop) {
                service_swapServices(&temp1, &temp2);
            }
        }
    }
}

/**
 * @brief Checks if provided data matches with current service
 * 
 * @param node current service pointer
 * @param priority_start start priority
 * @param priority_stop stop priority
 * @param name name
 * @return uint8_t 1 if matches, otherwise 0
 */
inline static uint8_t ifServiceMatches(struct service_node* node, uint16_t priority_start, uint16_t priority_stop, const char* name) {
    if (node->priority_start == priority_start && node->priority_stop == priority_stop && (strcmp(name, node->name) == 0))
        return 1;
    return 0;
}

/**
 * @brief List all registered services
 * 
 * @param node head
 */
static void listAll(struct service_node* node) {
    console_info("Registered services\r\n");
    while (node != NULL) {
        console_info("%s\r\n", node->name);
        console_debug("%s, start=%d, stop=%d\r\n", node->name, node->priority_start, node->priority_stop);
        node = node->next;
    }
    console_info("End of listing\r\n");
}

/**
 * @brief Check if service with given data exists
 * 
 * @param node head
 * @param name name of service
 * @return uint8_t 1 if exists, otherwise 0
 */
static uint8_t exists(struct service_node* node, const char* name) {
    uint8_t result = 0;
    while (node != NULL) {
        if (strcmp(name, node->name) == 0) {
            result = 1;
            break;
        }
        node = node->next;
    }
    return result;
}

/**
 * @brief Searches for the service by name
 * 
 * @param node service_head
 * @param name name of the service
 * @return struct service_node* pointer to the service if found, otherwise null
 */
static struct service_node* findByName(struct service_node* node, const char* name) {
    while (node != NULL) {
        if (strcmp(name, node->name) == 0) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

/**
 * @brief Register new service (only if it doesn't exist)
 * 
 * @param priority_start start priority of service
 * @param priority_stop stop priority of service
 * @param name name of service
 */
inline static void registerService(uint16_t priority_start, uint16_t priority_stop, const char* name) {
    if (!exists(service_head, name)) {
        addToList(&service_head, priority_start, priority_stop, name);
    }
}

// /**
//  * @brief Unregister service (only if it exists)
//  *
//  * @param priority priority of service
//  * @param name name of service
//  */
// static void service_unregister(uint16_t priority, const char* name) {
//     if (service_exists(service_head, priority, name)) {
//         service_remove_from_list(&service_head, priority, name);
//     }
// }

/**
 * @brief Checks if specified directory exists
 * 
 * @param dirpath char* Path to the directory
 * @return uint8_t 1 if exists
 */
inline static uint8_t dirExists(const char* dirpath) {
    if (util_dirExists(dirpath)) {
        return 1;
    } else {
        console_error("%s directory is missing. Services won't be started!\r\n", dirpath);
        return 0;
    }
}

/**
 * @brief Updates service indexed by name and priority with provided data 
 * 
 * @param head head of the list
 * @param pid pid of the service
 * @param time time when started
 * @param state state of the service
 * @param priority_start start priority of the service
 * @param priority_stop start priority of the service
 * @param name name of the service
 */
static void updateData(pid_t pid, time_t time, uint8_t state, uint16_t priority_start, uint16_t priority_stop, const char* name) {
    struct service_node* node = service_head;
    while (node != NULL) {
        if (ifServiceMatches(node, priority_start, priority_stop, name)) {
            node->pid = pid;
            node->time = time;
            node->state = state;
            return;
        }
        node = node->next;
    }
}

/**
 * @brief Updates service with provided data 
 * 
 * @param head head of the list
 * @param pid pid of the service
 * @param time time when started
 * @param state state of the service
 * @param priority_start start priority of the service
 * @param priority_stop start priority of the service
 * @param name name of the service
 */
static void updateDataByPointer(struct service_node** node, pid_t pid, time_t time, uint8_t state, uint16_t priority_start, uint16_t priority_stop, const char* name) {
    if (*node != NULL) {
        (*node)->pid = pid;
        (*node)->time = time;
        (*node)->state = state;
        (*node)->priority_start = priority_start;
        (*node)->priority_stop = priority_stop;
        strcpy((*node)->name, name);
    }
}

/**
 * @brief Scan /etc/quickinit/services/enabled for enabled services and register them
 * 
 * @return uint8_t EXIT_FAILURE or EXIT_SUCCESS
 */
static uint8_t scanAndAddEnabledServices() {
    DIR* d = opendir(ENABLED_DIR);

    if (d) {
        struct dirent* dir;
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG || dir->d_type == DT_LNK) {
                size_t namelen = strlen(dir->d_name);
                int16_t priority_start = 0, priority_stop = 0;
                char* prio_to_int_status = NULL;  // used for strtol checking

                if (namelen < 5) {
                    console_error("Skipped service! Filename isn't valid!\r\n");
                    continue;  // skip this service
                }

                char* buf = (char*)malloc((namelen + 1) * sizeof(char));  // alloc buffer for filename
                strcpy(buf, dir->d_name);                                 // copy name to a buffer

                if (buf[0] == 'S') {                                                      // check if first character of the file name is S (start)
                    char* buf_prio = (char*)malloc((namelen + 1) * sizeof(char));         // alloc start priority buffer
                    strcpy(buf_prio, buf + 1);                                            // copy buffer, skipping first character (S) into start priority buffer
                    buf_prio[3] = 0;                                                      // add \0 just after the number ends, so you can use the buffer with strtol
                    priority_start = (int16_t)strtol(buf_prio, &prio_to_int_status, 10);  // convert to integer base 10
                    free(buf_prio);
                } else if (buf[0] == 'K') {                                              // if first char is K
                    char* buf_prio = (char*)malloc((namelen + 1) * sizeof(char));        // alloc start priority buffer
                    strcpy(buf_prio, buf + 1);                                           // copy buffer, skipping first character (S) into start priority buffer
                    buf_prio[3] = 0;                                                     // add \0 just after the number ends, so you can use the buffer with strtol
                    priority_stop = (int16_t)strtol(buf_prio, &prio_to_int_status, 10);  // convert to integer base 10
                    free(buf_prio);
                }
                if (*prio_to_int_status != '\0') {
                    console_error("Skipped service! Priority isn't valid!\r\n");
                    continue;  // skip this service
                }

                if (priority_start != 0) {
                    if (!exists(service_head, buf + 4)) {
                        registerService((uint16_t)priority_start, 0, buf + 4);  // register service with priority, buf+3 is name without S or K and priority (4 characters)
                    } else {
                        struct service_node* tmp = findByName(service_head, buf + 4);
                        updateDataByPointer(&tmp, 0, 0, SERVICE_STOPPED, (uint16_t)priority_start, tmp->priority_stop, buf + 4);  // update service with proper data
                    }
                } else if (priority_stop != 0) {
                    if (!exists(service_head, buf + 4)) {
                        registerService(0, (uint16_t)priority_stop, buf + 4);
                    } else {
                        struct service_node* tmp = findByName(service_head, buf + 4);
                        updateDataByPointer(&tmp, 0, 0, SERVICE_STOPPED, tmp->priority_start, (uint16_t)priority_stop, buf + 4);
                    }
                }
                free(buf);
            }
        }
        closedir(d);
        return EXIT_SUCCESS;
    } else {
        return EXIT_FAILURE;
    }
}

/**
 * @brief Uses realpath to convert priority and name of the service to path of start executable
 * 
 * @param priority_start start priority of the service
 * @param name name of the service
 * @param buf output buffer with size of PATH_MAX
 * @return uint8_t EXIT_SUCCESS or EXIT_FAILURE
 */
static uint8_t findStartExec(uint16_t priority_start, const char* name, char* buf) {
    uint8_t ret = EXIT_SUCCESS;
    size_t path_size = snprintf(NULL, 0, "%s/S%03d%s", ENABLED_DIR, priority_start, name) + 1;
    char* path = (char*)malloc(path_size);
    sprintf(path, "%s/S%03d%s", ENABLED_DIR, priority_start, name);
    char* res = realpath(path, buf);

    if (!res)
        ret = EXIT_FAILURE;

    free(path);
    return ret;
}

/**
 * @brief Uses realpath to convert priority and name of the service to path of start executable
 * 
 * @param priority_stop stop priority of the service
 * @param name name of the service
 * @param buf output buffer with size of PATH_MAX
 * @return uint8_t EXIT_SUCCESS or EXIT_FAILURE
 */
static uint8_t findStopExec(uint16_t priority_stop, const char* name, char* buf) {
    uint8_t ret = EXIT_SUCCESS;
    size_t path_size = snprintf(NULL, 0, "%s/K%03d%s", ENABLED_DIR, priority_stop, name) + 1;
    char* path = (char*)malloc(path_size);
    sprintf(path, "%s/K%03d%s", ENABLED_DIR, priority_stop, name);
    char* res = realpath(path, buf);

    if (!res)
        ret = EXIT_FAILURE;

    free(path);
    return ret;
}

/**
 * @brief Starts all enabled services
 * 
 */
static void startEnabledServices() {
    sortServicesAscendingByStartPriority();

    struct service_node* temp = service_head;

    while (temp != NULL) {
        if (temp->priority_start == 0) {  // skip if don't need starting
            temp = temp->next;
            continue;
        }

        char resource[PATH_MAX];

        if (findStartExec(temp->priority_start, temp->name, resource) != EXIT_SUCCESS) {
            temp = temp->next;
            continue;
        }
        char* cmdbuf = (char*)malloc((strlen(resource) + strlen(" start") + 1) * sizeof(char));  // alloc buffer for command
        sprintf(cmdbuf, "%s start", resource);                                                   // append start to command buffer
        console_debug("%s \r\n", cmdbuf);

        pid_t pid = process_execute(cmdbuf);
        updateData(pid, time(NULL), SERVICE_STARTING, temp->priority_start, temp->priority_stop, temp->name);  // set status as starting
        free(cmdbuf);

        temp = temp->next;
    }
}

/**
 * @brief Stops all enabled services
 * 
 */
void svc_stopEnabledServices() {
    sortServicesAscendingByStopPriority();

    struct service_node* temp = service_head;

    while (temp != NULL) {
        if (temp->priority_stop == 0 || temp->state != SERVICE_STARTED) {  // skip if don't need stopping or it isn't running
            temp = temp->next;
            continue;
        }

        char resource[PATH_MAX];

        if (findStopExec(temp->priority_stop, temp->name, resource) != EXIT_SUCCESS) {
            temp = temp->next;
            continue;
        }
        char* cmdbuf = (char*)malloc((strlen(resource) + strlen(" stop") + 1) * sizeof(char));  // alloc buffer for command
        sprintf(cmdbuf, "%s stop", resource);
        console_debug("%s \r\n", cmdbuf);

        process_execute(cmdbuf);
        updateData(0, 0, SERVICE_STOPPED, temp->priority_start, temp->priority_stop, temp->name);
        free(cmdbuf);

        temp = temp->next;
    }
}

/**
 * @brief Wait until all services are running
 * 
 */
void svc_waitForAll() {
    struct service_node* node = service_head;
    while (node != NULL) {
        while (node->state == SERVICE_STARTING) {
            int stat;
            waitpid(node->pid, &stat, WUNTRACED);
            updateData(node->pid, node->time, SERVICE_STARTED, node->priority_start, node->priority_stop, node->name);  // change state to started
        }
        node = node->next;
    }
}

/**
 * @brief Initializes service spawning
 * 
 */
void svc_init() {
    if (!dirExists(AVAILABLE_DIR) && !dirExists(ENABLED_DIR)) {
        return;
    }
    scanAndAddEnabledServices();
    startEnabledServices();
    listAll(service_head);
}