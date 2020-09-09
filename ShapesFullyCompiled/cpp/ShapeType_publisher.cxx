/*
* (c) Copyright, Real-Time Innovations, 2012.  All rights reserved.
* RTI grants Licensee a license to use, modify, compile, and create derivative
* works of the software solely for use with RTI Connext DDS. Licensee may
* redistribute copies of the software provided that all such copies are subject
* to this license. The software is provided "as is", with no warranty of any
* type, including any warranty for fitness for any purpose. RTI is under no
* obligation to maintain or support the software. RTI shall not be liable for
* any incidental or consequential damages arising out of the use or inability
* to use the software.
*/

/* ShapeType_publisher.cxx

A publication of data of type ShapeTypeExtended

This file is derived from code automatically generated by the rtiddsgen 
command:

rtiddsgen -language C++ -example <arch> ShapeType.idl

Example publication of type ShapeTypeExtended automatically generated by 
'rtiddsgen'. To test it, follow these steps:

(1) Compile this file and the example subscription.

(2) Start the subscription

(3) Start the publication

(4) [Optional] Specify the list of discovery initial peers and 
multicast receive addresses via an environment variable or a file 
(in the current working directory) called NDDS_DISCOVERY_PEERS. 

You can run any number of publisher and subscriber programs, and can 
add and remove them dynamically from the domain.
*/

#include <stdio.h>
#include <stdlib.h>

#include "ShapeType.h"
#include "ShapeTypeSupport.h"
#include "ndds/ndds_cpp.h"

/* Delete all entities */
static int publisher_shutdown(
    DDSDomainParticipant *participant)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDSTheParticipantFactory->delete_participant(participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    /* RTI Connext provides finalize_instance() method on
    domain participant factory for people who want to release memory used
    by the participant factory. Uncomment the following block of code for
    clean destruction of the singleton. */
    /*

    retcode = DDSDomainParticipantFactory::finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "finalize_instance error %d\n", retcode);
        status = -1;
    }
    */

    return status;
}

extern "C" int publisher_main(int domainId, int sample_count)
{
    DDSDomainParticipant *participant = NULL;
    DDSPublisher *publisher = NULL;
    DDS_DomainParticipantQos participant_qos;
    DDSTopic *topic = NULL;
    DDSDataWriter *writer = NULL;
    ShapeTypeExtendedDataWriter * shapeTypeExtended_writer = NULL;
    ShapeTypeExtended *instance = NULL;
    DDS_ReturnCode_t retcode;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    const char *type_name = NULL;
    int count = 0;  
    DDS_Duration_t send_period = {0,200000000};

    /* To customize participant QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    participant = DDSTheParticipantFactory->create_participant(
        domainId, DDS_PARTICIPANT_QOS_DEFAULT, 
        NULL /* listener */, DDS_STATUS_MASK_NONE);
    /*
    DDSTheParticipantFactory->get_default_participant_qos(participant_qos);
    //Remove Shared Memory from default peers.
    participant_qos.discovery.initial_peers.length(2);
    participant_qos.discovery.initial_peers[0] =
        DDS_String_dup("builtin.udpv4://127.0.0.1");
    participant_qos.discovery.initial_peers[1] =
        DDS_String_dup("builtin.udpv4://239.255.0.1");
    participant = DDSTheParticipantFactory->create_participant(
                  domainId, participant_qos,
                 NULL,
                 DDS_STATUS_MASK_NONE); 
    */
    if (participant == NULL) {
        fprintf(stderr, "create_participant error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize publisher QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    publisher = participant->create_publisher(  
        DDS_PUBLISHER_QOS_DEFAULT, NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
        fprintf(stderr, "create_publisher error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* Register type before creating topic */
    type_name = ShapeTypeExtendedTypeSupport::get_type_name();
    retcode = ShapeTypeExtendedTypeSupport::register_type(
        participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "register_type error %d\n", retcode);
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize topic QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    topic = participant->create_topic(
        "Square",
        type_name, DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        fprintf(stderr, "create_topic error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize data writer QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    writer = publisher->create_datawriter(
        topic, DDS_DATAWRITER_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (writer == NULL) {
        fprintf(stderr, "create_datawriter error\n");
        publisher_shutdown(participant);
        return -1;
    }

    shapeTypeExtended_writer = ShapeTypeExtendedDataWriter::narrow(writer);
    if (shapeTypeExtended_writer == NULL) {
        fprintf(stderr, "DataWriter narrow error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* Create data sample for writing */
    instance = ShapeTypeExtendedTypeSupport::create_data();
    if (instance == NULL) {
        fprintf(stderr, "ShapeTypeExtendedTypeSupport::create_data error\n");
        publisher_shutdown(participant);
        return -1;
    }

    char *t_blue=(char *)"BLUE";
    instance->color=t_blue;
    instance->shapesize=30;

    /* For a data type that has a key, if the same instance is going to be
    written multiple times, initialize the key here
    and register the keyed instance prior to writing */

    // instance_handle = ShapeTypeExtended_writer->register_instance(*instance);
    int xdelta = 2;
	int ydelta = 5;
	int xmax = 250;
	int ymax = 250;
    /* Main loop */
    for (count=0; (sample_count == 0) || (count < sample_count); ++count) {

        instance->x += xdelta;
        if (instance->x > xmax || instance->x < 0) xdelta = xdelta * -1;
        instance->y += ydelta;
        if (instance->y > ymax || instance->y < 0) ydelta = ydelta * -1;

        printf("Writing ShapeTypeExtended, count %d\n", count);

        /* Modify the data to be sent here */

        retcode = shapeTypeExtended_writer->write(*instance, instance_handle);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "write error %d\n", retcode);
        }

        NDDSUtility::sleep(send_period);
    }
    /*
    retcode = shapeTypeExtended_writer->unregister_instance(
        *instance, instance_handle);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "unregister instance error %d\n", retcode);
    }
    */

    /* Delete data sample */
    retcode = ShapeTypeExtendedTypeSupport::delete_data(instance);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "ShapeTypeExtendedTypeSupport::delete_data error %d\n", retcode);
    }

    /* Delete all entities */
    return publisher_shutdown(participant);
}

int main(int argc, char *argv[])
{
    int domain_id = 0;
    int sample_count = 0; /* infinite loop */

    if (argc >= 2) {
        domain_id = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    /* Uncomment this to turn on additional logging
    NDDSConfigLogger::get_instance()->
    set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API, 
    NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */

    return publisher_main(domain_id, sample_count);
}
