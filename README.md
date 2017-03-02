# LM-ICP
Performs the [Iterative Closest Point (ICP)](http://www.vtk.org/doc/nightly/html/classvtkIterativeClosestPointTransform.html) algorithm and transfer point data scalars between two VTK surfaces

This program is for registering two VTK surfaces (source and target) and trasferring the source's Point data Scalars from source to the target. 

The usage of the program is given below: 

## Usage 
The usage is through command line: 
```
LMICP <source> 
      <target> 
      <pre_registration_matrix_txt> 
      <new_source_in_target_vtk_filename> 
      <new_target_with_source_scalars_vtk_filename>
```

## Parameters

The first two parameters ```source``` and ```target``` are the input and output VTK files. 

The next <pre_registration_matrix_txt> is simply a textfile containing each of the sixteen entries of the 4x4 matrix of registering the source to the target, if they are in completely different positions in space. Here is an example matrix: 
```
-0.279057
0.0734779
-1.32207
40.904
1.27442
-0.351746
-0.288549
73.3852
-0.359323
-1.30461
0.00333676
-241.703
0
0
0
1
```

The last two parameters are output filenames for VTK files that contain the program's output. 

Of particular importance is the last parameter which contains the target shell with the source data after it's being registered using matrix and ICP algorithm. 

## Author 

```
Dr. Rashed Karim 
Department of Biomedical Engineering 
King's College London 
```
