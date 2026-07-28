/*
 * mxwl/devicekit/DeviceKitDriverManager.c
 *
 * DriverKit Driver Manager implementation.
 * Manages driver registration, matching, and lifecycle.
 *
 * Copyright (c) 2026 MinSTEP Project
 */

#include "Headers/DeviceKit.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_DRIVERS 256
static DKitDriverRef _driver_registry[MAX_DRIVERS];
static int _driver_count = 0;
static DKitFramebufferDriverRef _framebuffer_driver = NULL;
static bool _initialized = false;

DKitDriverRef DKitCreateDriver(const char *className, uint32_t driverType)
{
    DKitDriver *driver = (DKitDriver *)malloc(sizeof(DKitDriver));
    if (!driver) return NULL;
    
    memset(driver, 0, sizeof(DKitDriver));
    driver->isa = NULL;
    driver->name = strdup(className ? className : "Unknown");
    driver->bundlePath = NULL;
    driver->state = DKIT_STATE_NOT_LOADED;
    driver->driverType = driverType;
    driver->platform = NULL;
    driver->userData = NULL;
    
    return driver;
}

int DKitDestroyDriver(DKitDriverRef driver)
{
    if (!driver) return DKIT_ERROR_GENERAL;
    
    if (driver->state == DKIT_STATE_READY) {
        /* Can't destroy active driver */
        return DKIT_ERROR_INVALID_STATE;
    }
    
    if (driver->name) free((void *)driver->name);
    if (driver->bundlePath) free((void *)driver->bundlePath);
    if (driver->userData) free(driver->userData);
    
    free(driver);
    return DKIT_SUCCESS;
}

int DKitRegisterDriver(DKitDriverRef driver)
{
    if (!driver) return DKIT_ERROR_GENERAL;
    
    if (DKitFindDriver(driver->name)) {
        return DKIT_ERROR_ALREADY_EXISTS;
    }
    
    if (_driver_count >= MAX_DRIVERS) {
        return DKIT_ERROR_GENERAL;
    }
    
    _driver_registry[_driver_count++] = driver;
    driver->state = DKIT_STATE_LOADED;
    
    return DKIT_SUCCESS;
}

int DKitUnregisterDriver(DKitDriverRef driver)
{
    if (!driver) return DKIT_ERROR_GENERAL;
    
    int i;
    for (i = 0; i < _driver_count; i++) {
        if (_driver_registry[i] == driver) {
            for (; i < _driver_count - 1; i++) {
                _driver_registry[i] = _driver_registry[i + 1];
            }
            _driver_count--;
            driver->state = DKIT_STATE_NOT_LOADED;
            return DKIT_SUCCESS;
        }
    }
    
    return DKIT_ERROR_NOT_FOUND;
}

DKitDriverRef DKitFindDriver(const char *name)
{
    if (!name) return NULL;
    
    int i;
    for (i = 0; i < _driver_count; i++) {
        if (_driver_registry[i] && 
            _driver_registry[i]->name &&
            strcmp(_driver_registry[i]->name, name) == 0) {
            return _driver_registry[i];
        }
    }
    
    return NULL;
}

DKitDriverRef DKitFindDriverByType(uint32_t driverType)
{
    int i;
    for (i = 0; i < _driver_count; i++) {
        if (_driver_registry[i] && 
            _driver_registry[i]->driverType == driverType &&
            _driver_registry[i]->state == DKIT_STATE_READY) {
            return _driver_registry[i];
        }
    }
    
    return NULL;
}

DKitDriverRef DKitGetFramebufferDriver(void)
{
    return (DKitDriverRef)_framebuffer_driver;
}

int DKitSetFramebufferDriver(DKitDriverRef driver)
{
    if (driver && driver->driverType != DKIT_DRIVER_TYPE_FRAMEBUFFER) {
        return DKIT_ERROR_GENERAL;
    }
    
    _framebuffer_driver = (DKitFramebufferDriverRef)driver;
    return DKIT_SUCCESS;
}

int DKitProbeDrivers(void)
{
    int i;
    int probed = 0;
    
    for (i = 0; i < _driver_count; i++) {
        DKitDriver *driver = _driver_registry[i];
        if (!driver) continue;
        
        if (driver->state == DKIT_STATE_LOADED && driver->probe) {
            int result = driver->probe(driver);
            if (result >= 0) {
                probed++;
            }
        }
    }
    
    return probed;
}

int DKitStartDrivers(void)
{
    int i;
    int started = 0;
    
    for (i = 0; i < _driver_count; i++) {
        DKitDriver *driver = _driver_registry[i];
        if (!driver) continue;
        
        if (driver->state == DKIT_STATE_LOADED && driver->start) {
            driver->state = DKIT_STATE_INITIALIZING;
            int result = driver->start(driver);
            if (result == DKIT_SUCCESS) {
                driver->state = DKIT_STATE_READY;
                started++;
            } else {
                driver->state = DKIT_STATE_ERROR;
            }
        }
    }
    
    return started;
}

int DKitStopDrivers(void)
{
    int i;
    int stopped = 0;
    
    for (i = _driver_count - 1; i >= 0; i--) {
        DKitDriver *driver = _driver_registry[i];
        if (!driver) continue;
        
        if (driver->state == DKIT_STATE_READY && driver->stop) {
            driver->state = DKIT_STATE_UNLOADING;
            int result = driver->stop(driver);
            if (result == DKIT_SUCCESS) {
                driver->state = DKIT_STATE_LOADED;
                stopped++;
            } else {
                driver->state = DKIT_STATE_ERROR;
            }
        }
    }
    
    return stopped;
}

int DKitInitialize(void)
{
    if (_initialized) {
        return DKIT_SUCCESS;
    }
    
    _driver_count = 0;
    _framebuffer_driver = NULL;
    _initialized = true;
    
    return DKIT_SUCCESS;
}

int DKitShutdown(void)
{
    if (!_initialized) {
        return DKIT_SUCCESS;
    }
    
    /* Stop all drivers first */
    DKitStopDrivers();
    
    /* Unregister all drivers */
    while (_driver_count > 0) {
        _driver_count--;
        DKitDriverRef driver = _driver_registry[_driver_count];
        if (driver) {
            DKitDestroyDriver(driver);
        }
        _driver_registry[_driver_count] = NULL;
    }
    
    _framebuffer_driver = NULL;
    _initialized = false;
    
    return DKIT_SUCCESS;
}

void DKitPrintDriverInfo(DKitDriverRef driver)
{
    if (!driver) {
        printf("Driver: NULL\n");
        return;
    }
    
    printf("Driver: %s\n", driver->name ? driver->name : "(null)");
    printf("  Type: 0x%04x\n", driver->driverType);
    
    const char *state_str = "unknown";
    switch (driver->state) {
        case DKIT_STATE_NOT_LOADED:    state_str = "not loaded"; break;
        case DKIT_STATE_LOADING:      state_str = "loading"; break;
        case DKIT_STATE_LOADED:      state_str = "loaded"; break;
        case DKIT_STATE_INITIALIZING: state_str = "initializing"; break;
        case DKIT_STATE_READY:       state_str = "ready"; break;
        case DKIT_STATE_SUSPENDED:  state_str = "suspended"; break;
        case DKIT_STATE_ERROR:      state_str = "error"; break;
        case DKIT_STATE_UNLOADING:   state_str = "unloading"; break;
    }
    printf("  State: %s\n", state_str);
}

void DKitPrintDriverRegistry(void)
{
    printf("DriverKit Driver Registry\n");
    printf("=========================\n");
    printf("Total drivers: %d\n\n", _driver_count);
    
    int i;
    for (i = 0; i < _driver_count; i++) {
        DKitDriver *driver = _driver_registry[i];
        printf("[%d] ", i);
        DKitPrintDriverInfo(driver);
        printf("\n");
    }
    
    if (_framebuffer_driver) {
        printf("Framebuffer Driver: %s\n", 
               _framebuffer_driver->base.name ? 
               _framebuffer_driver->base.name : "(null)");
    }
}