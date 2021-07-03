/**
 * @file svc.h
 * @author bwisn (bwisn_dev (at) outlook (dot) com)
 * @brief 
 * @version 0.1
 * @date 24-06-2021
 * 
 * 
 */
#ifndef SVC_H_INCLUDED
#define SVC_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

/**
 * @brief Maximum length of service name
 * 
 */
#define SERVICENAME_MAXLEN 128

/*! \cond PRIVATE */
#define SERVICE_STOPPED 0
#define SERVICE_STARTED 1
#define SERVICE_STARTING 2
/*! \endcond */

/**
 * @brief Service structure
 * 
 */
struct service_node {
    /**
     * @brief PID of the started service
     * 
     */
    pid_t pid;

    /**
     * @brief Timestamp when the service was started
     * 
     */
    time_t time;

    /**
     * @brief Status of the service
     * 
     */
    uint8_t state;

    /**
     * @brief Startup priority of the service
     * 
     */
    uint16_t priority_start;

    /**
     * @brief Stop priority of the service (at shutdown)
     * 
     */
    uint16_t priority_stop;

    /**
     * @brief Name of the service
     * 
     */
    char name[SERVICENAME_MAXLEN];

    /**
     * @brief Next service pointer
     * 
     */
    struct service_node* next;
};

void svc_init();
void svc_waitForAll();
void svc_stopEnabledServices();
#endif