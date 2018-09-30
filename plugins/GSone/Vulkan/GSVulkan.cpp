/*  GSOne
 *  Copyright (C) 2004-2018 PCSX2 Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "GSVulkan.h"
#include <vector>
#include <set>

vulkan_context nimoy;

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
{
    GSLog::WriteLn("Validation layer: %s", pCallbackData->pMessage);

    return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator) 
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(nimoy.instance, "vkCreateDebugUtilsMessengerEXT");

    if (func != nullptr) 
    {
        return func(nimoy.instance, pCreateInfo, pAllocator, &nimoy.callback);
    } 
    else 
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator) 
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(nimoy.instance, "vkDestroyDebugUtilsMessengerEXT");

    if (func != nullptr) 
    {
        func(nimoy.instance, nimoy.callback, pAllocator);
    }
}

 void setupDebugCallback() 
 {
    VkResult res;
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
        
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    res = CreateDebugUtilsMessengerEXT(&createInfo, nullptr);
    if (res != VK_SUCCESS) 
    {
        GSLog::WriteLn("Failed to set up debug callback! (%s)", VkResultToString(res));
    }
}

std::set<std::string> get_supported_extensions() 
{
    uint32_t count;

    //Get number of extensions.
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr); 

    //Pull list.
    std::vector<VkExtensionProperties> extensions(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()); 

    std::set<std::string> results;
    for (auto & extension : extensions) 
    {
        results.insert(extension.extensionName);
    }
    return results;
}

bool CreateInstance()
{
    const char *layers[] = { "VK_LAYER_LUNARG_standard_validation" };
    VkResult res;

    // initialize the VkApplicationInfo structure
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "PCSX2";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "PCSX2";
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_API_VERSION_1_1;

    // initialize the VkInstanceCreateInfo structure
    VkInstanceCreateInfo inst_info = {};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = NULL;
    inst_info.flags = 0;
    inst_info.pApplicationInfo = &app_info;
    inst_info.enabledExtensionCount = 0;
    inst_info.ppEnabledExtensionNames = NULL;

    if (nimoy.enableValidationLayers)
    {
        inst_info.enabledLayerCount = 1;
        inst_info.ppEnabledLayerNames = layers;
    }
    else
    {
        inst_info.enabledLayerCount = 0;
        inst_info.ppEnabledLayerNames = NULL;
    }


    res = vkCreateInstance(&inst_info, NULL, &nimoy.instance);

    if (res != VK_SUCCESS) 
    {
        GSLog::WriteLn("Vulkan instance not created! (%s)", VkResultToString(res));
        return false;
    } 

    GSLog::WriteLn("Vulkan instance created.");
    return true;
}

void PrintDeviceInfo(VkPhysicalDevice physicalDevice)
{
    uint32_t queueFamilyCount = 0;
    VkPhysicalDeviceProperties deviceProperties;
    

    memset(&deviceProperties, 0, sizeof deviceProperties);
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    GSLog::WriteLn("Driver Version: %d", deviceProperties.driverVersion);
    GSLog::WriteLn("Device Name:    %s", deviceProperties.deviceName);
    GSLog::WriteLn("Device Type:    %d", deviceProperties.deviceType);
    GSLog::WriteLn("API Version:    %d.%d.%d",
        VK_VER_MAJOR(deviceProperties.apiVersion),
        VK_VER_MINOR(deviceProperties.apiVersion),
        VK_VER_PATCH(deviceProperties.apiVersion));

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);

    std::vector<VkQueueFamilyProperties>familyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, &familyProperties[0]);

    // Print the families
    for (u32 j = 0; j < queueFamilyCount; j++) 
    {
        GSLog::WriteLn("Count of Queues: %d", familyProperties[j].queueCount);
        GSLog::WriteLn("Supported operations on this queue:");

        if (familyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) GSLog::WriteLn("\t\t Graphics");
        if (familyProperties[j].queueFlags & VK_QUEUE_COMPUTE_BIT) GSLog::WriteLn("\t\t Compute");
        if (familyProperties[j].queueFlags & VK_QUEUE_TRANSFER_BIT) GSLog::WriteLn("\t\t Transfer");
        if (familyProperties[j].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) GSLog::WriteLn("\t\t Sparse Binding");
        }
}

bool CheckDevices()
{
    // Query how many devices are present in the system
    uint32_t deviceCount = 0;

    // The first time, we call it to get the number of devices.
    VkResult res = vkEnumeratePhysicalDevices(nimoy.instance, &deviceCount, NULL);
    if (res != VK_SUCCESS) 
    {
        GSLog::WriteLn("Failed to query the number of physical devices present. (%s)", VkResultToString(res));
        return false;
    }

    // There has to be at least one device present
    if (deviceCount == 0) 
    {
        GSLog::WriteLn("Couldn't detect any device present with Vulkan support. (%s)", VkResultToString(res));
        return false;
    }

    // Get the physical devices with the number gotten from the first call.
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    res = vkEnumeratePhysicalDevices(nimoy.instance, &deviceCount, &physicalDevices[0]);
    if (res != VK_SUCCESS) 
    {
        GSLog::WriteLn("Failed to enumerate physical devices present. (%s)", VkResultToString(res));
        return false;
    }

     GSLog::WriteLn("Number of physical devices present: %d", deviceCount);

    for (uint32_t i = 0; i < deviceCount; i++) 
    {
        PrintDeviceInfo(physicalDevices[i]);
    }

    return true;
}

bool CheckValidationLayers()
{
    uint32_t layerCount = 0;
    bool foundValidation = false;

    // Check once to get the count.
    vkEnumerateInstanceLayerProperties( &layerCount, NULL );

    VkLayerProperties *layersAvailable = new VkLayerProperties[layerCount];
    vkEnumerateInstanceLayerProperties( &layerCount, layersAvailable );

    for (uint32_t i = 0; i < layerCount; ++i) 
    {
        if (strcmp( layersAvailable[i].layerName, "VK_LAYER_LUNARG_standard_validation" ) == 0) 
        {
            foundValidation = true;
        }
    }

    if (!foundValidation)
    {
            GSLog::WriteLn("Could not find validation layer.");
            return false;
    }
    
    GSLog::WriteLn("Found validation layer.");
    return true;
}
void InitVulkan()
{
    bool ret = false;
    
    GSLog::WriteLn("Initializing Vulkan! (Sort of.)");

    nimoy.enableValidationLayers = CheckValidationLayers();

    ret = CreateInstance(); // Create it with validation layers if they are there, otherwise don't use them.
    
    if (nimoy.enableValidationLayers) setupDebugCallback();

    if (ret) ret = CheckDevices();

    if (ret)
        GSLog::WriteLn("Vulkan initialized.");
    else
        GSLog::WriteLn("Vulkan not initialized.");
}

void DestroyVulkan()
{
    if (nimoy.enableValidationLayers) 
    {
        DestroyDebugUtilsMessengerEXT(nullptr);
    }

    vkDestroyInstance(nimoy.instance, NULL);
}