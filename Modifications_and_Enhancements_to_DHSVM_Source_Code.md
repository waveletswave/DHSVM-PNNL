# Modifications and Enhancements to DHSVM Source Code

## Introduction

This document outlines the specific modifications and enhancements made to the Distributed Hydrology Soil Vegetation Model (DHSVM) source code. These changes were implemented to improve functionality, add new capabilities, and ensure the model meets the latest requirements for hydrological simulations.

## New Capabilities and Enhancements

### 1. Addition of Chiwawa Watershed Test Case

- **File Added:** `TestCase/Chiwawa/input/chiwawa.dhs`
- **Description:** A new test case for the Chiwawa watershed was added to the DHSVM test suite. This includes input data configurations for running simulations specific to the Chiwawa watershed.

### 2. Improved Streamflow Plotting

- **Python Script Added:** `scripts/plot_streamflow.py`
- **Description:** A Python script was developed to visualize the streamflow output data. This script reads the streamflow data from the `Streamflow.Only` file located in the output directory of the test case and generates a plot showing streamflow over time. The script includes:
  - Reading and parsing of streamflow data.
  - Generation of time-series plots using Matplotlib.
  - Handling of various data formats and ensuring compatibility with the existing DHSVM output structure.

### 3. Enhancements to Build Configuration

- **Changes Made:**
  - Enhanced CMake configuration to support new test cases and scripts.
  - Improved compatibility with different system libraries and ensured smooth integration with Python scripts.
  - Detailed build instructions were refined to include new dependencies and test cases.

## Modifications to Existing Files

### 1. Updates to DHSVM Source Code

- **Files Modified:**
  - `DHSVM/sourcecode/DHSVMChannel.c`
  - `DHSVM/sourcecode/fixroads.c`
- **Description:**

#### `DHSVMChannel.c`:

- **Modification Summary:**

  - Enhanced the initialization and routing of stream and road networks.
  - Added functionality to read and process new types of input files.
  - Improved error handling and logging for better debugging and traceability.
- **Key Changes:**

  - **InitChannel Function:**
    - Enhanced the function to initialize road and stream networks by reading additional input files.
    - Added conditions to handle riparian vegetation parameters when `Options->StreamTemp` is enabled.
  - **RouteChannel Function:**
    - Improved the logic to handle surface water distribution to roads without sinks.
    - Added functionality to compute and add culvert outflow to surface water.
    - Enhanced routing of stream channels and saving of results to output files.
  - **Other Functions:**
    - Updated `ChannelCulvertFlow`, `ChannelCut`, and `ChannelFraction` functions to support new calculations and improve performance.

#### `fixroads.c`:

- **Modification Summary:**

  - Improved the logic to handle elevation data and stream crossings for road segments.
  - Enhanced memory allocation and deallocation to ensure efficient handling of large datasets.
  - Refined the process for identifying and managing road sinks based on elevation and stream intersection data.
- **Key Changes:**

  - **Main Function:**
    - Enhanced file reading and parsing to support various input formats.
    - Improved logic to determine road sinks based on minimum elevation and stream intersections.
    - Added detailed logging for better traceability and debugging.
  - **Memory Management:**
    - Improved allocation and deallocation of memory to handle large datasets efficiently.
  - **Output Handling:**
    - Refined the process to generate and write output files with updated road and stream data.

## Running the Chiwawa Watershed Test Case

```sh
cd /Users/benthosyy/DHSVM-PNNL/TestCase/Chiwawa/output
./DHSVM/sourcecode/DHSVM /Users/benthosyy/DHSVM-PNNL/TestCase/Chiwawa/input/chiwawa.dhs
```

## Visualizing Streamflow Output

```sh
cd /Users/benthosyy/DHSVM-PNNL/TestCase/Chiwawa/output
python ../../scripts/plot_streamflow.py
```
