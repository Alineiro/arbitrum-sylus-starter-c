//LIST OF ERRORS AND IMPROVEMENTS
//
//CANNOT PASS NAME TO REGISTER_DEVELOPER, ALWAYS RETURNS INPUT TOO LONG
//MAYBE DUE TO THAT, THE SAME ADDRESS CAN BE REGISTERED AS DEV MULTIPLE TIMES
//NEED TO CHECK IF DEVS ARE STORED CORRECTLY
//NEED TO ADD 1 TO NONCE COUNT IN PLAYGROUND TO FIX INCORRECT NONCE ISSUE
//VERIFY_APP_HASH FUNCTION DOESNT WORK CORRECTLY, SHOULD RETURN DEVELOPER ADDRESS, RETURNS EMPTY ADDRESS
//
//COULD PROPERLY ADD STORAGE, SO THAT EACH DEV CAN SUBMIT MULTIPLE APPS, AND THEIR SUBMITTED APPS CAN BE CHECKED

#include "../include/stylus_sdk.h"
#include "../stylus-sdk-c/include/hostio.h"
#include "../stylus-sdk-c/include/stylus_utils.h"
#include "../stylus-sdk-c/include/storage.h"
#include "../stylus-sdk-c/include/string.h"
#include "../stylus-sdk-c/include/stylus_debug.h"

// Storage slots for developers and app hashes
#define STORAGE_SLOT__developers 0x0
#define STORAGE_SLOT__appHashes 0x1

// Buffer for temporary output
uint8_t buf_out[32];

void storage_combine_keys(uint8_t *base, uint8_t *suffix, uint8_t *output) {
    for (int i = 0; i < 32; i++) {
        output[i] = base[i] ^ suffix[i]; // XOR byte-by-byte
    }
}

/*
void truncate_to_20_bytes(uint8_t *input, uint8_t *output) {
    memcpy(output, input, 20); // Copy only the first 20 bytes
}
*/

// Helper function to generate a key for app hashes
void generate_app_key(uint8_t *appHash, uint8_t *output) {
  // Combine the base slot with the hash to create a unique key
  storage_combine_keys((uint8_t *)STORAGE_SLOT__appHashes, appHash, output);
}

/*
uint8_t* get_sender() {
    static uint8_t sender[20] = {0x01, 0x02, 0x03}; // Mock sender address (20 bytes)
    return sender;
}
*/

// Mock implementation to get the sender address
uint8_t* get_sender() {
    static uint8_t sender[32] = {0x01, 0x02, 0x03}; // Mock sender address
    return sender;
}

// Construct and return a success result
ArbResult _return_success_bebi32(uint8_t *retval) {
    ArbResult res;
    res.status = Success;
    res.output = retval;
    res.output_len = 32;
    return res;
}

// Register a developer (stores their address as the key and name as the value)
ArbResult register_developer(uint8_t *input, size_t len) {
  if (len > 32) {
    return _return_short_string(Failure, "InputTooLong");
  }

  uint8_t *developer_address = get_sender(); // Get the caller's address
  //uint8_t developer_address[20];
  //truncate_to_20_bytes(get_sender(), developer_address);
  uint8_t slot[32];
  storage_combine_keys((uint8_t *)STORAGE_SLOT__developers, developer_address, slot);

  // Check if already registered
  storage_load_bytes32(slot, buf_out);
  if (!bebi32_is_zero(buf_out)) {
    return _return_short_string(Failure, "AlreadyRegistered");
  }

  // Register developer by storing their name
  storage_cache_bytes32(slot, input);
  storage_flush_cache(false);

  return _return_short_string(Success, "Registered");
}

ArbResult submit_app_hash(uint8_t *input, size_t len) {
    if (len != 64) {
        return _return_short_string(Failure, "InvalidInputLength");
    }

    uint8_t developer_address[32];
    uint8_t app_hash[32];

    // Split input into developer address and app hash
    memcpy(developer_address, input, 32);
    memcpy(app_hash, input + 32, 32);

    // Check if the app hash already exists
    uint8_t app_slot[32];
    generate_app_key(app_hash, app_slot);
    storage_load_bytes32(app_slot, buf_out);

    if (!bebi32_is_zero(buf_out)) {
        return _return_short_string(Failure, "HashExists");
    }

    // Link the app hash to the developer address
    storage_cache_bytes32(app_slot, developer_address);
    storage_flush_cache(false);

    return _return_short_string(Success, "AppHashStored");
}


// Verify an app hash
ArbResult verify_app_hash(uint8_t *input, size_t len) {
  if (len != 32) {
    return _return_short_string(Failure, "InvalidHashLength");
  }

  uint8_t slot[32];
  generate_app_key(input, slot);

  // Load the developer address linked to the app hash
  storage_load_bytes32(slot, buf_out);
  //if (bebi32_is_zero(buf_out)) {
  //  return _return_short_string(Failure, "NotFound");
  //}

  // Return the developer address
  return _return_success_bebi32(buf_out);
}

// Handler to route function calls
int handler(size_t argc) {
  uint8_t argv[argc];
  read_args(argv);

  FunctionRegistry registry[] = {
      {to_function_selector("register_developer(string)"), register_developer},
      {to_function_selector("submit_app_hash(address,bytes32)"), submit_app_hash},
      {to_function_selector("verify_app_hash(bytes32)"), verify_app_hash},
  };

  uint32_t signature = *((uint32_t *)argv);
  ArbResult res = call_function(
      registry,
      sizeof(registry) / sizeof(registry[0]),
      signature,
      argv + 4,
      argc - 4);
  return (write_result(res.output, res.output_len), res.status);
}

ENTRYPOINT(handler)


///////////////////////////////////////

/*#include "../include/stylus_sdk.h"
#include "../stylus-sdk-c/include/hostio.h"
#include "../stylus-sdk-c/include/stylus_utils.h"
#include "../stylus-sdk-c/include/storage.h"
#include "../stylus-sdk-c/include/string.h"

// Storage slots for developers and app hashes
#define STORAGE_SLOT__developers 0x0
#define STORAGE_SLOT__appHashes 0x1

// Buffer for temporary output
uint8_t buf_out[32];

// Function declarations (if missing in included headers)
//void storage_combine_keys(const uint8_t *base, const uint8_t *suffix, uint8_t *output);
//uint8_t* get_sender(); // Get the caller's address
//ArbResult _return_success_bebi32(const uint8_t *retval);

// Helper function to generate a key for app hashes
void generate_app_key(uint8_t *appHash, uint8_t *output) {
  // Combine the base slot with the hash to create a unique key
  storage_combine_keys((uint8_t *)STORAGE_SLOT__appHashes, appHash, output);
}

ArbResult _return_success_bebi32(uint8_t *retval) {
    ArbResult res;
    res.status = Success;        // Operation succeeded
    res.output = retval;         // Set the return value
    res.output_len = 32;         // Assume 32-byte return value
    return res;
}

//FIND SOME WAY TO FIND AN ALTERNATIVE TO THE STORAGECOMBINEKEY FUNCTION
//EITHER MAKE A COMPLETELY NEW FUNCTION, OR FIND A WORKAROUND, IF NOT CHANGE IDEA
//AND APPROACH, NEED TO UNDERSTAND FUNCTIONALITY AND LIBRARIES

//void storage_combine_keys(const uint8_t *base, const uint8_t *suffix, uint8_t *output) {
    // Hash the base and suffix together to create a unique key
//    hash2(base, suffix, output);
//}

uint8_t* get_sender() {
    static uint8_t sender[32]; // Buffer to hold the sender address
    msg_sender(sender);        // Fetch the sender's address into the buffer
    return sender;
}

// Register a developer (stores their address as the key and name as the value)
ArbResult register_developer(uint8_t *input, size_t len) {
  if (len > 32) {
    return _return_short_string(Failure, "InputTooLong");
  }

  uint8_t *developer_address = get_sender(); // Get the caller's address
  uint8_t slot[32];
  storage_combine_keys((uint8_t *)STORAGE_SLOT__developers, developer_address, slot);

  // Check if already registered
  storage_load_bytes32(slot, buf_out);
  if (!bebi32_is_zero(buf_out)) {
    return _return_short_string(Failure, "AlreadyRegistered");
  }

  // Register developer by storing their name
  storage_cache_bytes32(slot, input);
  storage_flush_cache(false);

  return _return_short_string(Success, "Registered");
}

// Submit an app hash
ArbResult submit_app_hash(uint8_t *input, size_t len) {
  if (len != 64) {
    return _return_short_string(Failure, "InvalidInputLength");
  }

  uint8_t developer_address[32];
  uint8_t app_hash[32];

  // Extract developer and app hash from input
  memcpy(developer_address, input, 32);
  memcpy(app_hash, input + 32, 32);

  uint8_t slot[32];
  storage_combine_keys((uint8_t *)STORAGE_SLOT__developers, developer_address, slot);

  // Verify developer is registered
  storage_load_bytes32(slot, buf_out);
  if (bebi32_is_zero(buf_out)) {
    return _return_short_string(Failure, "NotRegistered");
  }

  uint8_t app_slot[32];
  generate_app_key(app_hash, app_slot);

  // Verify if the app hash is already submitted
  storage_load_bytes32(app_slot, buf_out);
  if (!bebi32_is_zero(buf_out)) {
    return _return_short_string(Failure, "HashExists");
  }

  // Link the app hash to the developer
  storage_cache_bytes32(app_slot, developer_address);
  storage_flush_cache(false);

  return _return_short_string(Success, "AppHashStored");
}

// Verify an app hash
ArbResult verify_app_hash(uint8_t *input, size_t len) {
  if (len != 32) {
    return _return_short_string(Failure, "InvalidHashLength");
  }

  uint8_t slot[32];
  generate_app_key(input, slot);

  // Load the developer address linked to the app hash
  storage_load_bytes32(slot, buf_out);
  if (bebi32_is_zero(buf_out)) {
    return _return_short_string(Failure, "NotFound");
  }

  // Return the developer address
  return _return_success_bebi32(buf_out);
}

// Handler to route function calls
int handler(size_t argc) {
  uint8_t argv[argc];
  read_args(argv);

  FunctionRegistry registry[] = {
      {to_function_selector("register_developer(string)"), register_developer},
      {to_function_selector("submit_app_hash(address,bytes32)"), submit_app_hash},
      {to_function_selector("verify_app_hash(bytes32)"), verify_app_hash},
      //{to_function_selector("set_value(uint256)"), set_value},
      //{to_function_selector("get_value()"), get_value},
  };

  uint32_t signature = *((uint32_t *)argv);
  ArbResult res = call_function(
      registry,
      sizeof(registry) / sizeof(registry[0]),
      signature,
      argv + 4,
      argc - 4);
  return (write_result(res.output, res.output_len), res.status);
}

ENTRYPOINT(handler)
*/

////////////////////////////////
////////////////////////////////
//
//
//ORIGINAL INTEGRATION IS:
//
////////////////////////////////


/*
#include "../include/stylus_sdk.h"
#include "../stylus-sdk-c/include/stylus_utils.h"
#include "../stylus-sdk-c/include/storage.h"
#include "../stylus-sdk-c/include/string.h"

#define STORAGE_SLOT__value 0x0

//
// General utils/helpers
//

// buffer used to write output, avoiding malloc
uint8_t buf_out[32];

// succeed and return a bebi32
ArbResult inline _return_success_bebi32(bebi32 const retval)
{
  ArbResult res = {Success, retval, 32};
  return res;
}

ArbResult set_value(uint8_t *input, size_t len)
{

  if (len != 32)
  {
    // revert if input length is not 32 bytes
    return _return_short_string(Failure, "InvalidLength");
  }

  uint8_t *slot_address = (uint8_t *)(STORAGE_SLOT__value + 0); // Get the slot address

  // Allocate a temporary buffer to store the input
  storage_cache_bytes32(slot_address, input);

  // Flush the cache to store the value permanently
  storage_flush_cache(false);
  return _return_success_bebi32(input);
}

ArbResult get_value(uint8_t *input, size_t len)
{

  uint8_t *slot_address = (uint8_t *)(STORAGE_SLOT__value + 0); // Get the slot address

  storage_load_bytes32(slot_address, buf_out);
  if (bebi32_is_zero(buf_out))
  {
    return _return_short_string(Failure, "NotSet");
  }

  return _return_success_bebi32(buf_out);
}

ArbResult dumbtest(uint8_t *input, size_t len) {
    int a = 5;
    int b = 10;
    int sum = a + b;

    // Buffer for the resulting string
    char result[12]; // Enough for the string representation of an int
    int i = 0;

    // Handle negative numbers
    bool is_negative = false;
    if (sum < 0) {
        is_negative = true;
        sum = -sum;
    }

    // Convert number to string (in reverse order)
    do {
        result[i++] = (sum % 10) + '0';
        sum /= 10;
    } while (sum > 0);

    // Add negative sign if needed
    if (is_negative) {
        result[i++] = '-';
    }

    // Null-terminate the string
    result[i] = '\0';

    // Reverse the string to get the correct order
    for (int j = 0; j < i / 2; j++) {
        char temp = result[j];
        result[j] = result[i - j - 1];
        result[i - j - 1] = temp;
    }

    return _return_short_string(Success, result);
}

ArbResult	hello_world(uint8_t *input, size_t len)
{
	return _return_short_string(Success, "pizza con piña y aguacate");
}

int handler(size_t argc)
{
  // Save the function calldata
  uint8_t argv[argc];
  read_args(argv); // 4 bytes for selector + function arguments

  // Define the registry array with registered functions
  FunctionRegistry registry[] = {
      {to_function_selector("set_value(uint256)"), set_value},
      {to_function_selector("get_value()"), get_value},
      {to_function_selector("hola_mundo()"), hello_world},
      {to_function_selector("dumbtest()"), dumbtest},
      // Add more functions as needed here
  };

  uint32_t signature = *((uint32_t *)argv); // Parse function selector

  // Call the function based on the signature
  ArbResult res = call_function(registry,
                                sizeof(registry) / sizeof(registry[0]),
                                signature, argv + 4, argc - 4 // Exclude the selector from calldata
  );
  return (write_result(res.output, res.output_len), res.status);
}

ENTRYPOINT(handler)
*/
