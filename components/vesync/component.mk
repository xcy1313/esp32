
COMPONENT_ADD_INCLUDEDIRS += api service module module/encryptor driver hal
COMPONENT_SRCDIRS := api service module module/encryptor driver hal
COMPONENT_EMBED_TXTFILES := service/certificate/mqtt_ca.crt service/certificate/mqtt_client.crt service/certificate/mqtt_client.key
COMPONENT_EMBED_TXTFILES += service/certificate/https_ca.crt service/certificate/https_client.crt service/certificate/https_client.key
