import pandas as pd
import matplotlib.pyplot as plt

# Define the file path
file_path = '/Users/benthosyy/DHSVM-PNNL/TestCase/Chiwawa/output/Streamflow.Only'

# Read the data
streamflow = pd.read_csv(file_path, delimiter='\s+', skiprows=1, names=['DATE', 'Chiwawa'], parse_dates=['DATE'])

# Plot the streamflow
plt.figure(figsize=(12, 6))
plt.plot(streamflow['DATE'], streamflow['Chiwawa'], label='Streamflow')
plt.xlabel('Date')
plt.ylabel('Streamflow (cfs)')
plt.title('Chiwawa Streamflow Over Time')
plt.legend()
plt.grid(True)
plt.show()