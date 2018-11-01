#include "docker-images-json.h"
#include <json-c/json_tokener.h>
#include <json-c/json.h>

static void JsonParse(json_object *jobj);
static GHashTable *JsonHash;
static char *JsonValueHandle(json_object *jobj)
{
    enum json_type type;
    type = json_object_get_type(jobj); /*Getting the type of the json object*/
    switch (type) 
    {   
        case json_type_boolean: 
        case json_type_double:
        case json_type_int: 
            break;
        case json_type_string:
            return json_object_get_string(jobj);
        default:
            break;
    }
    return NULL;
}
static void JsonArrayHandle( json_object *jobj, char *key) 
{
    enum json_type type;
    int arraylen;
    json_object *jarray ;
    int i;
    json_object * jvalue;
        
    jarray = jobj; /*Simply get the array*/

    if(key) 
    {   
        /*Getting the array if it is a key value pair */
        jarray = json_object_object_get(jobj, key); 
    }   
    /* Getting the length of the array */
    arraylen = json_object_array_length(jarray);
    for (i=0; i< arraylen; i++)
    {  
        /* Getting the array element at position i  */
        jvalue = json_object_array_get_idx(jarray, i); 
        type = json_object_get_type(jvalue);
        if (type == json_type_array) 
        {
            JsonArrayHandle(jvalue, NULL);
        }    
        else
        {
            JsonParse(jvalue);
        }
    }

}
                                                   
static void JsonParse(json_object *jobj)
{
    enum json_type type;
    json_object * j;
    char *value;

    /*ergodic*/
    json_object_object_foreach(jobj, key, val)  
    {
        type = json_object_get_type(val);
        switch (type)
        {
            case json_type_boolean:
            case json_type_double:
            case json_type_int:
            case json_type_string:
                 value = JsonValueHandle(val);
                 if(value == NULL)
                     break;
                 g_hash_table_insert(JsonHash,key,
                                     g_slist_append(g_hash_table_lookup(JsonHash,key),
                                      value));

                    break;
            case json_type_object:
                if(strcmp(key,"labels") == 0)
                    break;
                j = json_object_object_get(jobj, key);
                JsonParse(j);
                    break;
            case json_type_array:
                JsonArrayHandle(jobj, key);
                    break;
            case json_type_null:
                    break;
            default:
                    break;
        }
    }
}

GHashTable *AnalyticJson(const char *data)
{
    if(JsonHash != NULL)
         g_hash_table_destroy(JsonHash);   
    JsonHash = g_hash_table_new(g_str_hash, g_str_equal);
    JsonParse(json_tokener_parse(data));
    
    return JsonHash;
}    
