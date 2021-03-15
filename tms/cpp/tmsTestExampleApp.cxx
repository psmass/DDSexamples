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

/* tmsTestExample_publisher.cxx

A publication of data of type tms_MicrogridMembershipApproval

This file is derived from code automatically generated by the rtiddsgen 
command:

rtiddsgen -language C++ -example <arch> tmsTestExample.idl

Example publication of type tms_MicrogridMembershipApproval automatically generated by 
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
#include <string>
#include <signal.h>
#include <iostream>

#include "ndds/ndds_cpp.h"
#include <pthread.h>
#include "tmsTestExample.h" //note this file uses static const vs #defs using gratuitous space, but is derived from the official TMS datamodel


static bool run_flag = true;

// Local prototypes
void handle_SIGINT(int unused);
int tms_app_main (unsigned int);
int main (int argc, char *argv[]);

//-------------------------------------------------------------------
// handle_SIGINT - sets flag for orderly shutdown on Ctrl-C
//-------------------------------------------------------------------

void handle_SIGINT(int unused)
{
  // On CTRL+C - abort! //
  run_flag = false;
}


class WaitsetWriterInfo {
    // holds waitset info needed for the writer waitset processing thread
    public:
        WaitsetWriterInfo(std::string writerName) {
            myName = writerName;
        }
        std::string me(){ return myName; }

        DDSDynamicDataWriter * writer;
		bool * run_flag;
    private:
        std::string myName;
} ;

void*  pthreadToProcWriterEvents(void  * waitsetWriterInfo) {
	WaitsetWriterInfo * myWaitsetInfo;
    myWaitsetInfo = (WaitsetWriterInfo *)waitsetWriterInfo;
	DDSWaitSet * waitset = waitset = new DDSWaitSet();;
    DDS_ReturnCode_t retcode;
    DDSConditionSeq active_conditions_seq;

    std::cout << "\nCreated Writer Pthread: " << myWaitsetInfo->me() << std::endl;

    // Configure Waitset for Writer Status ****
    DDSStatusCondition *status_condition = myWaitsetInfo->writer->get_statuscondition();
    if (status_condition == NULL) {
        printf("Writer thread: get_statuscondition error\n");
        goto end_writer_thread;
    }

    // Set enabled statuses
    retcode = status_condition->set_enabled_statuses(
            DDS_PUBLICATION_MATCHED_STATUS);
    if (retcode != DDS_RETCODE_OK) {
        printf("Writer thread: set_enabled_statuses error\n");
        goto end_writer_thread;
    }

    // Attach Status Conditions to the above waitset
    retcode = waitset->attach_condition(status_condition);
    if (retcode != DDS_RETCODE_OK) {
        printf("Writer thread: attach_condition error\n");
        goto end_writer_thread;
    }

    // wait() blocks execution of the thread until one or more attached condition triggers  
	// thread exits upon ^c or error
    while ((* myWaitsetInfo->run_flag) == true) { 
        retcode = waitset->wait(active_conditions_seq, DDS_DURATION_INFINITE);
        /* We get to timeout if no conditions were triggered */
        if (retcode == DDS_RETCODE_TIMEOUT) {
            printf("Writer thread: Wait timed out!! No conditions were triggered.\n");
            continue;
        } else if (retcode != DDS_RETCODE_OK) {
            printf("Writer thread: wait returned error: %d\n", retcode);
            goto end_writer_thread;
        }

        /* Get the number of active conditions */
        int active_conditions = active_conditions_seq.length();

        for (int i = 0; i < active_conditions; ++i) {
            /* Compare with Status Conditions */
            if (active_conditions_seq[i] == status_condition) {
                DDS_StatusMask triggeredmask =
                        myWaitsetInfo->writer->get_status_changes();

                if (triggeredmask & DDS_PUBLICATION_MATCHED_STATUS) {
					DDS_PublicationMatchedStatus st;
                	myWaitsetInfo->writer->get_publication_matched_status(st);
					std::cout << myWaitsetInfo->me() << "Writer Subs: " 
                    << st.current_count << "  " << st.current_count_change << std::endl;
                }
            } else {
                // writers can only have status condition
                std::cout << myWaitsetInfo->me() << " Writer: False Writer Event Trigger" << std::endl;
            }
        }
	} // While (run_flag)
	end_writer_thread: // reached by ^C or an error
	std::cout << myWaitsetInfo->me() << " Writer: Pthread Exiting"<< std::endl;
	exit(0);
}


class WaitsetReaderInfo {
    // holds waitset info needed for the Reader waitset processing thread
    public:
        WaitsetReaderInfo(std::string readerName) {
            myName = readerName;
        };
        std::string me() {return myName;}

        DDSDynamicDataReader * reader;
		bool * run_flag;
    private:
        std::string myName;
} ;

void*  pthreadToProcReaderEvents(void *waitsetReaderInfo) {
    WaitsetReaderInfo * myWaitsetInfo;
    myWaitsetInfo = (WaitsetReaderInfo *)waitsetReaderInfo;
	DDSStatusCondition *status_condition =  NULL;
	DDSReadCondition * read_condition = NULL;
	DDSWaitSet *waitset = new DDSWaitSet();
    DDS_ReturnCode_t retcode;
    DDSConditionSeq active_conditions_seq;
	DDS_DynamicDataSeq data_seq;
	DDS_SampleInfoSeq info_seq;

    std::cout << "\nCreated Reader Pthread: " << myWaitsetInfo->me() << std::endl;

    // Create read condition
    read_condition = myWaitsetInfo->reader->create_readcondition(
        DDS_NOT_READ_SAMPLE_STATE,
        DDS_ANY_VIEW_STATE,
        DDS_ANY_INSTANCE_STATE);
    if (read_condition == NULL) {
        printf("Reader thread: create_readcondition error\n");
		goto end_reader_thread;
    }

    //  Get status conditions
    status_condition = myWaitsetInfo->reader->get_statuscondition();
    if (status_condition == NULL) {
        printf("Reader thread: get_statuscondition error\n");
 		goto end_reader_thread;
    }

    // Set enabled statuses
    retcode = status_condition->set_enabled_statuses(DDS_SUBSCRIPTION_MATCHED_STATUS);
    if (retcode != DDS_RETCODE_OK) {
        printf("Reader thread: set_enabled_statuses error\n");
 		goto end_reader_thread;
    }   

    /* Attach Read Conditions */
    retcode = waitset->attach_condition(read_condition);
    if (retcode != DDS_RETCODE_OK) {
        printf("Reader thread: attach_condition error\n");
		goto end_reader_thread;
    }

    /* Attach Status Conditions */
    retcode = waitset->attach_condition(status_condition);
    if (retcode != DDS_RETCODE_OK) {
        printf("Reader thread: attach_condition error\n");
		goto end_reader_thread;
    }

	while ((* myWaitsetInfo->run_flag) == true) {
       	retcode = waitset->wait(active_conditions_seq, DDS_DURATION_INFINITE);
        if (retcode == DDS_RETCODE_TIMEOUT) {
            printf("Reader thread: Wait timed out!! No conditions were triggered.\n");
            continue;
        } else if (retcode != DDS_RETCODE_OK) {
            printf("Reader thread:  wait returned error: %d\n", retcode);
            goto end_reader_thread;
        }

        int active_conditions = active_conditions_seq.length();

        for (int i = 0; i < active_conditions; ++i) {
            if (active_conditions_seq[i] == status_condition) {
                /* Get the status changes so we can check which status
                 * condition triggered. */
                DDS_StatusMask triggeredmask =
                        myWaitsetInfo->reader->get_status_changes();

                /* Subscription matched */
                if (triggeredmask & DDS_SUBSCRIPTION_MATCHED_STATUS) {
                    DDS_SubscriptionMatchedStatus st;
                    myWaitsetInfo->reader->get_subscription_matched_status(st);
                    std::cout << myWaitsetInfo->me() << "Reader Pubs: " 
                    << st.current_count << "  " << st.current_count_change << std::endl;
                }
            } else if (active_conditions_seq[i] == read_condition) { 
                // Get the latest samples
				retcode = myWaitsetInfo->reader->take(
							data_seq, info_seq, DDS_LENGTH_UNLIMITED,
							DDS_ANY_SAMPLE_STATE, DDS_ANY_VIEW_STATE, DDS_ANY_INSTANCE_STATE);
				if (retcode == DDS_RETCODE_OK) {
					for (int i = 0; i < data_seq.length(); ++i) {
						if (info_seq[i].valid_data) {                  
							//retcode=data_seq[i].get_ulong(myTopicInfo.field1, 
									//"Field1Name", DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
							if (retcode != DDS_RETCODE_OK) goto end_reader_thread;
							//retcode=data_seq[i].get_ulong(myTopicInfo.field2,
									//"Field2Name", DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
							if (retcode != DDS_RETCODE_OK) goto end_reader_thread;
							//std::cout << myWaitsetInfo->me() << " Field1" 
                            //<< myTopicInfo.field1  
                            //<< " Field2 " << myTopicInfo.field2
                            //<< std::endl;
						}
					}
				} else if (retcode == DDS_RETCODE_NO_DATA) {
					continue;
				} else {
					fprintf(stderr, "Reader thread: read data error %d\n", retcode);
					goto end_reader_thread;
				}
                retcode = myWaitsetInfo->reader->return_loan(data_seq, info_seq);
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr, "Reader thread:return_loan error %d\n", retcode);
                    goto end_reader_thread;
                }  
			}
		}
	} // While (run_flag)
	end_reader_thread: // reached by ^C or an error
	std::cout << myWaitsetInfo->me() << " Reader: Pthread Exiting"<< std::endl;
	exit(0);
}

/* Delete all entities */
static int participant_shutdown(
    DDSDomainParticipant *participant)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK) {
            std::cerr <<  "delete_contained_entities error " << retcode << std::endl << std::flush;
            status = -1;
        }

        retcode = DDSTheParticipantFactory->delete_participant(participant);
        if (retcode != DDS_RETCODE_OK) {
            std::cerr <<  "delete_participant error " << retcode << std::endl << std::flush;
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

extern "C" int tms_app_main(int sample_count)
{
    DDSDomainParticipant * participant = NULL;
	DDSDynamicDataWriter * device_announcement_writer = NULL;
	DDSDynamicDataWriter * microgrid_membership_request_writer = NULL;
	DDSDynamicDataReader * microgrid_membership_outcome_reader = NULL;
    DDS_DynamicData * product_info_data = NULL;
    DDS_DynamicData * microgrid_membership_request_data = NULL;
    DDS_ReturnCode_t retcode;

    char this_device_id [tms_LEN_Fingerprint] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};

    unsigned long long count = 0;  
    DDS_Duration_t send_period = {1,0};

    // Declare Reader and Writer thread Information structs
    WaitsetWriterInfo * myWaitsetDeviceAnnouncementWriterInfo = new WaitsetWriterInfo("DeviceAnnouncementTopic"); 
	WaitsetWriterInfo * myWaitsetMicrogridMembershipRequestWriterInfo = new WaitsetWriterInfo("MicrogridMembershipRequestTopic"); 
    WaitsetReaderInfo * myWaitsetMicrogridMembershipOutcomeReaderInfo = new WaitsetReaderInfo("MicrogridMembershipOutcomeTopic"); 

    /* To customize participant QoS, use 
    the configuration file USER_QOS_PROFILES.xml */
    std::cout << "Starting tms application\n" << std::flush;

    participant = DDSTheParticipantFactory->
            create_participant_from_config(
                                "TMS_ParticipantLibrary1::TMS_Participant1");
    if (participant == NULL) {
        std::cerr << "create_participant_from_config error " << std::endl << std::flush;
        participant_shutdown(participant);
        return -1;
    }
    
    std::cout << "Successfully Created Tactical Microgrid Participant from the System Designer config file"
     << std::endl << std::flush;
    
	device_announcement_writer = DDSDynamicDataWriter::narrow(
        participant->lookup_datawriter_by_name("TMS_Publisher1::DeviceAnnouncementTopicWriter"));
    if (device_announcement_writer  == NULL) {
        std::cerr << "TMS_Publisher1::DeviceAnnouncementTopicWriter: lookup_datawriter_by_name error "
        << retcode << std::endl << std::flush; 
		goto tms_app_main_end;
    }
    std::cout << "Successfully Found: TMS_Publisher1::MicrogridDeviceAnnouncementTopicWriter" 
    << std::endl << std::flush;

    microgrid_membership_request_writer = DDSDynamicDataWriter::narrow(
		// Defined only in domain_participant_library. PUblisher name not defined QoS file
        participant->lookup_datawriter_by_name("TMS_Publisher1::MicrogridMembershipRequestTopicWriter"));
    if (microgrid_membership_request_writer  == NULL) {
        std::cerr << "TMS_Publisher1::MicrogridMembershipRequestTopicWriter lookup_datawriter_by_name error " 
        << retcode << std::endl << std::flush; 
		goto tms_app_main_end;
    }
    std::cout << "Successfully Found: TMS_Publisher1::MicrogridMembershipRequestTopicWriter" 
    << std::endl << std::flush;

 	microgrid_membership_outcome_reader = DDSDynamicDataReader::narrow(
		// Defined only in domain_participant_library. PUblisher name not defined QoS file
		participant->lookup_datareader_by_name("TMS_Subscriber1::MicrogridMembershipOutcomeTopicReader")); 
    if (microgrid_membership_outcome_reader == NULL) {
        std::cerr << "TMS_Publisher1::MicrogridMembershipOutcomeTopicReader"
        << retcode << std::endl << std::flush;
		goto tms_app_main_end;
    } 
    std::cout << "Successfully Found: MS_Publisher1::MicrogridMembershipOutcomeTopicReader" 
    << std::endl << std::flush;   

    product_info_data = device_announcement_writer->create_data(DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
    if (product_info_data == NULL) {
        std::cerr << "product_info_data: create_data error"
        << retcode << std::endl << std::flush;
		goto tms_app_main_end;
    } 

    std::cout << "Successfully created: product_info_data topic w/device announcemenet writer" 
    << std::endl << std::flush;  

	// Turn up a waitset threads and hang on them for writer events and reader events and data
    myWaitsetDeviceAnnouncementWriterInfo->writer = device_announcement_writer;
	myWaitsetDeviceAnnouncementWriterInfo->run_flag = &run_flag;
    pthread_t wda_tid; // writer device_announcement tid
    pthread_create(&wda_tid, NULL, pthreadToProcWriterEvents, (void*) myWaitsetDeviceAnnouncementWriterInfo);

    myWaitsetMicrogridMembershipRequestWriterInfo->writer = microgrid_membership_request_writer;
	myWaitsetMicrogridMembershipRequestWriterInfo->run_flag = &run_flag;
    pthread_t wmmr_tid; // writer microgrid_membership_request tid
    pthread_create(&wmmr_tid, NULL, pthreadToProcWriterEvents, (void*) myWaitsetMicrogridMembershipRequestWriterInfo);
    
    myWaitsetMicrogridMembershipOutcomeReaderInfo->reader = microgrid_membership_outcome_reader;
	myWaitsetMicrogridMembershipOutcomeReaderInfo->run_flag = &run_flag;
    pthread_t rmmo_tid; // writer microgrid_membership_outcome tid
    pthread_create(&rmmo_tid, NULL, pthreadToProcReaderEvents, (void*) myWaitsetMicrogridMembershipOutcomeReaderInfo);
    
    /* Main loop */
    while (run_flag) {

        std::cout <<  tms_TOPIC_DEVICE_ANNOUNCEMENT << ": "<< count << std::endl << std::flush;

        product_info_data->set_octet_array("deviceId", DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED, tms_LEN_Fingerprint, (const DDS_Octet *)&this_device_id); 
        retcode = device_announcement_writer->write(* product_info_data, DDS_HANDLE_NIL);
        if (retcode != DDS_RETCODE_OK) {
            std::cerr << "writer: write error " << std::endl << std::flush;
            break;
        }
                    
        count++;
        std:sprintf(this_device_id, "%llu", count);
        /* Modify the data to be sent here */

        NDDSUtility::sleep(send_period);
    }

    tms_app_main_end:
    /* Delete all entities */
    std::cout << "Stopping - shutting down participant\n" << std::flush;

    return participant_shutdown(participant);
}

int main(int argc, char *argv[])
{
    int sample_count = 0; /* infinite loop */
    signal(SIGINT, handle_SIGINT);

    if (argc >= 2) {
        sample_count = atoi(argv[2]);
    }

    /* Uncomment this to turn on additional logging
    NDDSConfigLogger::get_instance()->
    set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API, 
    NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */

    return tms_app_main(sample_count);
}
