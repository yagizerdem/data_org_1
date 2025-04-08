#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemastypes.h>
#include <json-c/json.h>


typedef struct record record; 
typedef struct tag tag;
struct json_object* get_key_value(struct json_object* rootObj, const char* key);
int compare_by(const void *a, const void *b);

struct record{
    char device_id[8];
    char timestamp[32];
    float temperature;
    uint8_t humidity;
    char status[8];
    char location[31];
    char alert_level[10];
    uint8_t battery;
    char firmware_ver[16];
    uint8_t event_code;     
};

struct tag{
    char device_id[10];
    char timestamp[10];
    char temperature[12];
    char humidity[9];
    char status[7];
    char location[9];
    char alert_level[12];
    char battery[8];
    char firmware_ver[14];
    char event_code[11];      
};


tag tagHead;


// data from json file
char dataFileName[100];
int keyStart ;
int keyEnd ;
char order[100] ;
char *inputFile;
char *outputFile;
char *datFile;
char *conversionType;
char *seperator;


static const long hextable[] = { 
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1, 0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};


char* to_big_endian(int input){
    char* hex = (char*)malloc(9 * sizeof(char));
    sprintf(hex, "%08x", input); 
    return hex;
}

void swap(char* a, char* b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

char* concat(const char *s1, const char *s2)
 {
     char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
     // in real code you would check for errors in malloc here
     strcpy(result, s1);
     strcat(result, s2);
     return result;
 }

char* to_little_endian(int input){
    char* hex = (char*)malloc(9 * sizeof(char));
    sprintf(hex, "%08x", input); 
    for (int i = 0; i < 4; i+=2) {
        swap( &hex[i] , &hex[7 - i - 1 ]);
        swap( &hex[i+1] , &hex[7 - i]);
    }
    hex[8] = '\0';
    return hex;
}

long hexdec(unsigned const char *hex) {
    long ret = 0; 
    while (*hex && ret >= 0) {
       ret = (ret << 4) | hextable[*hex++];
    }
    return ret; 
 }

void toBinary(FILE *fp){
    char csvRow[9999];
    int counter = 0;

    record *home_device;
    home_device = (record *) calloc(1 , sizeof(record));

    while(fgets(csvRow, 999, fp)) {
        if(counter == 0){
            strcpy(tagHead.device_id,     strtok(csvRow, seperator));
            strcpy(tagHead.timestamp,     strtok(NULL, seperator));
            strcpy(tagHead.temperature,   strtok(NULL, seperator));
            strcpy(tagHead.humidity,      strtok(NULL, seperator));
            strcpy(tagHead.status,        strtok(NULL, seperator));
            strcpy(tagHead.location,      strtok(NULL, seperator));
            strcpy(tagHead.alert_level,   strtok(NULL, seperator));
            strcpy(tagHead.battery,       strtok(NULL, seperator));
            strcpy(tagHead.firmware_ver,  strtok(NULL, seperator));
            strcpy(tagHead.event_code,    strtok(NULL, seperator));
        }else{
            
            home_device = (record*) realloc(home_device, (counter) * sizeof(record)); //reallocate in every loop
            memset(&home_device[counter-1], 0, sizeof(record));
            char *device_id = strtok(csvRow, seperator);
            if (device_id != NULL) {
                strcpy(home_device[counter-1].device_id, device_id);
            }
            else{
                strcpy(home_device[counter-1].device_id, "N/A");
            }

            char *timestamp = strtok(NULL, seperator);
            if (timestamp != NULL) {
                strcpy(home_device[counter-1].timestamp, timestamp);
            }
            else{
                strcpy(home_device[counter-1].timestamp, "N/A");
            }

            char *temperature = strtok(NULL, seperator);
            if (temperature != NULL) {
                home_device[counter-1].temperature = atof(temperature);
            }



            char *humidity = strtok(NULL, seperator);
            if (humidity != NULL) {
                home_device[counter-1].humidity = (uint8_t)atoi(humidity); 
            }


            char *status = strtok(NULL, seperator);
            if (status != NULL) {
                strcpy(home_device[counter-1].status, status);
            }
            else{
                strcpy(home_device[counter-1].status, "N/A");
            }

            char *location = strtok(NULL, seperator);
            if (location != NULL) {
                strcpy(home_device[counter-1].location, location);
            }
            else{
                strcpy(home_device[counter-1].location, "N/A");
            }

            char *alert_level = strtok(NULL, seperator);
            if (alert_level != NULL) {
                strcpy(home_device[counter-1].alert_level, alert_level);
            }
            else{
                strcpy(home_device[counter-1].alert_level, "N/A");
            }

            char *battery = strtok(NULL, seperator);
            if (battery != NULL) {
                home_device[counter-1].battery = (uint8_t)atoi(battery);
            }

            char *firmware_ver = strtok(NULL, seperator);
            if (firmware_ver != NULL) {
                strcpy(home_device[counter-1].firmware_ver, firmware_ver);
            }
            else{
                strcpy(home_device[counter-1].firmware_ver, "N/A");
            }

            char *event_code = strtok(NULL, seperator);
            if (event_code != NULL) {
                home_device[counter-1].event_code = (uint8_t)atoi(event_code);  
            }
            
                    
        }

        counter += 1;
      }

      FILE *fp_write;
      fp_write = fopen(dataFileName,"wb"); //open a stream with r, w, a modes
      if(fp_write == NULL)
      {
          puts("error opening file!");
          exit (1);
      }


      for(int i = 0; i < counter -1 ; i++){
        fwrite(&home_device[i], sizeof(record), 1, fp_write);
      }

      fclose(fp_write);
    }

void BintoXML(){
    FILE *fp;
    fp=fopen(dataFileName,"rb");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }

    record* home_device = NULL;
    int counter = 0; 
    while (1) { 
        home_device = (record *) realloc(home_device, (counter + 1) * sizeof(record));
        if (home_device == NULL) {
            perror("Memory allocation failed");
            fclose(fp);
            return;
        }
    
        size_t items_read = fread(&home_device[counter], sizeof(record), 1, fp);
        if (items_read != 1) {
            // Either EOF or error
            break;
        }
    
        counter++;
    }

    qsort(home_device, counter, sizeof(struct record), compare_by);

    xmlDocPtr doc = NULL;
    xmlNodePtr root_node = NULL;
    doc = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "smartlogs");
    xmlDocSetRootElement(doc, root_node);

    for(int i = 0; i < counter  ; i++){
        xmlNodePtr entryNode = xmlNewChild(root_node, NULL, BAD_CAST "entry",BAD_CAST "");
        char id[10];
        snprintf(id, sizeof(id), "%d", i + 1);
        xmlNewProp(entryNode, BAD_CAST "id", BAD_CAST id);
        
        xmlNodePtr deviceNode = xmlNewChild(entryNode, NULL, BAD_CAST "device", BAD_CAST "");
        xmlNewChild(deviceNode, NULL, BAD_CAST tagHead.device_id, BAD_CAST home_device[i].device_id);
        xmlNewChild(deviceNode, NULL, BAD_CAST tagHead.location, BAD_CAST home_device[i].location);
        xmlNewChild(deviceNode, NULL, BAD_CAST tagHead.firmware_ver, BAD_CAST home_device[i].firmware_ver);
    
        
        xmlNodePtr metricsNode = xmlNewChild(entryNode, NULL, BAD_CAST "metrics", BAD_CAST "");
        xmlNewProp(metricsNode, BAD_CAST "status", BAD_CAST home_device[i].status );
        xmlNewProp(metricsNode, BAD_CAST "alert_level", BAD_CAST home_device[i].alert_level );
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", home_device[i].temperature);
        xmlNewChild(metricsNode, NULL, BAD_CAST "temperature", BAD_CAST buf);
        snprintf(buf, sizeof(buf), "%u", home_device[i].humidity);
        xmlNewChild(metricsNode, NULL, BAD_CAST tagHead.humidity, BAD_CAST buf);
        snprintf(buf, sizeof(buf), "%u", home_device[i].battery);
        xmlNewChild(metricsNode, NULL, BAD_CAST tagHead.battery, BAD_CAST buf);

        xmlNewChild(entryNode, NULL, BAD_CAST tagHead.timestamp, BAD_CAST home_device[i].timestamp);
        snprintf(buf, sizeof(buf), "%u", home_device[i].event_code);
        xmlNodePtr eventCodeNode = xmlNewChild(entryNode, NULL, BAD_CAST tagHead.event_code, BAD_CAST buf);
        char* big_endian = to_big_endian(home_device[i].event_code);
        char* little_endian = to_little_endian(home_device[i].event_code);
        xmlNewProp(eventCodeNode, BAD_CAST "littleEndian", BAD_CAST little_endian);
        xmlNewProp(eventCodeNode, BAD_CAST "bigEndian", BAD_CAST big_endian);
        long int value = hexdec(little_endian);
        snprintf(buf, sizeof(buf), "%ld", value);
        xmlNewProp(eventCodeNode, BAD_CAST "decimal", buf);
    }


    xmlSaveFormatFileEnc(outputFile, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    fclose(fp);
}

void readFile() {
    FILE *fp;
    char *fullPath = concat("./", inputFile);
    printf("%s" , fullPath);
    fp=fopen(fullPath,"r");
    if (fp == NULL) {
        perror("Error opening file");
        return;
    }
    toBinary(fp);
    fclose(fp);
    free(fullPath);
}

void validate(){
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
	
    char *XMLFileName = inputFile; // write your xml file here
    char *XSDFileName = outputFile; // write your xsd file here
    
    
    xmlLineNumbersDefault(1); //set line numbers, 0> no substitution, 1>substitution
    ctxt = xmlSchemaNewParserCtxt(XSDFileName); //create an xml schemas parse context
    schema = xmlSchemaParse(ctxt); //parse a schema definition resource and build an internal XML schema
    xmlSchemaFreeParserCtxt(ctxt); //free the resources associated to the schema parser context
    
    doc = xmlReadFile(XMLFileName, NULL, 0); //parse an XML file
    if (doc == NULL)
    {
        fprintf(stderr, "Could not parse %s\n", XMLFileName);
    }
    else
    {
        xmlSchemaValidCtxtPtr ctxt;  //structure xmlSchemaValidCtxt, not public by API
        int ret;
        
        ctxt = xmlSchemaNewValidCtxt(schema); //create an xml schemas validation context 
        ret = xmlSchemaValidateDoc(ctxt, doc); //validate a document tree in memory
        if (ret == 0) //validated
        {
            printf("%s validates\n", XMLFileName);
        }
        else if (ret > 0) //positive error code number
        {
            printf("%s fails to validate\n", XMLFileName);
        }
        else //internal or API error
        {
            printf("%s validation generated an internal error\n", XMLFileName);
        }
        xmlSchemaFreeValidCtxt(ctxt); //free the resources associated to the schema validation context
        xmlFreeDoc(doc);
    }
    // free the resource
    if(schema != NULL)
        xmlSchemaFree(schema); //deallocate a schema structure

    xmlSchemaCleanupTypes(); //cleanup the default xml schemas types library
    xmlCleanupParser(); //cleans memory allocated by the library itself 
    xmlMemoryDump(); //memory dump
    
}

void readJson(){
    FILE *fp = fopen("setupParams.json", "r");
    if (!fp) {
        perror("Unable to open file");
        return;
    }
    char * buffer = 0;
    long length;

    
      fseek (fp, 0, SEEK_END);
      length = ftell (fp);
      fseek (fp, 0, SEEK_SET);
      buffer = malloc (length);
      if (buffer)
      {
        fread (buffer, 1, length, fp);
      }
      fclose(fp);
    if (buffer)
    {

        struct json_object *jobj;
        jobj = json_tokener_parse(buffer);
        printf("jobj from str:\n---\n%s\n---\n", json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));
        

        json_object*  dataFileName_obj = get_key_value(jobj , "dataFileName");
        json_object*  order_obj = get_key_value(jobj , "order");
        json_object*  keystart_obj = get_key_value(jobj , "keyStart");
        json_object*  keyend_obj = get_key_value(jobj , "keyEnd");
        strcpy(dataFileName , json_object_get_string(dataFileName_obj));
        strcpy(order , json_object_get_string(order_obj));
        keyStart = json_object_get_int(keystart_obj);
        keyEnd = json_object_get_int(keyend_obj);
        
    }

}

json_object* get_key_value(json_object* rootObj, const char* key)
{
    json_object* returnObj;
    if (json_object_object_get_ex(rootObj, key, &returnObj))
    {
        return returnObj;
    }
    return NULL;
}

int compare_by(const void *a, const void *b) {
    const struct record *ra = (const struct record *)a;
    const struct record *rb = (const struct record *)b;
    
    int length = keyEnd - keyStart +1;
    uint8_t *buffer_a = malloc(length);
    memcpy(buffer_a, (uint8_t *)ra + keyStart , length);
    uint8_t *buffer_b = malloc(length);
    memcpy(buffer_b, (uint8_t *)rb + keyStart , length);

    size_t min_size = (sizeof(buffer_a) < sizeof(buffer_b)) ? sizeof(buffer_a) : sizeof(buffer_b);
    int is_bigger = 0;
    for (int i = 0; i < min_size/ sizeof(uint8_t); i++) {
        if (buffer_a[i] > buffer_b[i]) {
            is_bigger = 1;
            break;
        } else if (buffer_a[i] < buffer_b[i]) {
            is_bigger = -1;
            break;
        }
    }
    if (sizeof(buffer_a) > sizeof(buffer_b)) is_bigger =  1;
    if (sizeof(buffer_b) > sizeof(buffer_a)) is_bigger = -1;

    if (strcmp(order, "ASC") == 0) return  is_bigger;
    return -1 * is_bigger;
}


void print_instructions() {
    printf("\n-- compile --\n");
    printf("gcc program.c -o program $(xml2-config --cflags --libs) -ljson-c\n");

    printf("\n-- run program --\n");

    printf("\n1. convert to binary\n");
    printf("    ./program <csv file name> <binary file> 1 -seperator <1|2|3>\n");
    printf("    ./program smartlogs.csv logdata.dat 1 -seperator 1\n");
    printf("    ./program smartlogs.csv logdata.dat 1 -seperator 2\n");
    printf("    ./program smartlogs.csv logdata.dat 1 -seperator 3\n");

    printf("\n2. convert to xml\n");
    printf("    ./program <output file name> <binary file> 2\n");
    printf("    ./program out.xml logdata.dat 2\n");

    printf("\n3. validate xml with xsd\n");
    printf("    ./program <xml file name> <xsd file name> 3\n");
    printf("    ./program out.xml validation.xsd 3\n");

    printf("\n-- happy path --\n");
    printf("    - use csv with ','\n\n");

    printf("    run :\n");
    printf("        ./program smartlogs.csv logdata.dat 1 -seperator 1\n");
    printf("        ./program out.xml logdata.dat 2\n");
    printf("        ./program out.xml validation.xsd 3\n\n");
}


int main(int argc, char *argv[]) {
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            printf("Help:\n");
            print_instructions();
            return 0; 
        }
    }


    if(argc == 6){
        conversionType = argv[3];   
        printf("Conversion type is 1\n");
        inputFile = argv[1];
        datFile = argv[2];


        if(strcmp(argv[5] ,"1") == 0){
            seperator = ",";
        }
        if(strcmp(argv[5] ,"2") == 0){
            seperator = " ";
        }
        if(strcmp(argv[5] ,"3") == 0){
            seperator = ";";
        }
        
    }
    else if(argc ==4){
        conversionType = argv[3];
        if (strcmp(conversionType, "2") == 0){
            outputFile= argv[1];
            datFile = argv[2];
        }

        if (strcmp(conversionType, "3") == 0) {
            inputFile = argv[1];
            outputFile = argv[2];
        }
    }

    readJson();

    if (strcmp(conversionType, "1")== 0){
        printf("convert csv to binary \n ");
        printf("input file name %s \n ", inputFile);
        printf("datfile file name %s \n", datFile);
        printf("seperator %s \n", seperator);
        readFile();
    }
    if (strcmp(conversionType, "2") == 0){
        printf("convert binary to xml \n ");
        printf("outputFile file name %s \n ", outputFile);
        printf("dat file name %s \n", datFile);


        strcpy(tagHead.device_id, "device_id");
        strcpy(tagHead.timestamp, "timestamp");
        strcpy(tagHead.temperature, "temperature");
        strcpy(tagHead.humidity, "humidity");
        strcpy(tagHead.status, "status");
        strcpy(tagHead.location, "location");
        strcpy(tagHead.alert_level, "alert_level");
        strcpy(tagHead.battery, "battery");
        strcpy(tagHead.firmware_ver, "firmware_ver");
        strcpy(tagHead.event_code, "event_code");

        BintoXML();
    }
    if (strcmp(conversionType, "3") == 0){
        printf("xsd validation \n");
        printf("xml file name %s \n ", inputFile);
        printf("xsd file name %s \n", outputFile);
        validate();
    }


    return 0;
}