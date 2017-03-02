// STL
#include <iostream>

// VTK
#include <vtkXMLPolyDataWriter.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTransform.h>
#include <vtkLandmarkTransform.h>
#include <vtkIterativeClosestPointTransform.h>
#include <vtkPolyDataWriter.h>
#include <vtkPolyDataReader.h>
#include <vtkPointLocator.h>
#include <vtkFloatArray.h>
#include <vtkGenericCell.h>
#include <vtkPointData.h>
#include <vtkKdTreePointLocator.h>
#include <vtkFileOutputWindow.h>



void ReadMatrix4x4(const char* fn, vtkMatrix4x4* matrix)
{
	ifstream in(fn); 
	int m = 4, n = 4; 
	double element = 0; 

	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			in >> element; 
			matrix->SetElement(i, j, element);
		}
		
	}
	
	in.close();
}

void TransferPointDataToTargetPoly(vtkSmartPointer<vtkPolyData> source, vtkSmartPointer<vtkPolyData> target, vtkSmartPointer<vtkDataArray> source_scalars)
{

	double xyz_target[3];

	vtkIdType closestPointID = -1;

//	vtkSmartPointer<vtkFloatArray> source_scalars = vtkSmartPointer<vtkFloatArray>::New();
	vtkSmartPointer<vtkFloatArray> target_scalars = vtkSmartPointer<vtkFloatArray>::New();

	//source_scalars = vtkFloatArray::SafeDownCast(source->GetPointData()->GetScalars());

	//cout << "scalars in source = " << source_scalars->GetNumberOfTuples() << endl;


	// vtkPointLocator is a spatial search object to quickly locate points in 3D 
	// Quick search is possible by dividing a specified region of space into a regular array of rectangular buckets
	vtkSmartPointer<vtkPointLocator> point_locator = vtkSmartPointer<vtkPointLocator>::New();
	point_locator->SetDataSet(source);
	point_locator->AutomaticOn();
	point_locator->BuildLocator();


	for (int i = 0; i<target->GetNumberOfPoints(); i++)
	{
		target->GetPoint(i, xyz_target);
		closestPointID = -1;

		closestPointID = point_locator->FindClosestPoint(xyz_target);

		if (closestPointID > 0)
		{

			if (closestPointID < source_scalars->GetNumberOfTuples())
			{
				double d = source_scalars->GetTuple1(closestPointID);
				target_scalars->InsertNextTuple1(d);	// data from nearest neighbour 
			}

		}

	}

	target->GetPointData()->SetScalars(target_scalars);

}

void ICPRegistration(vtkPolyData* source, vtkPolyData* target, vtkSmartPointer<vtkPolyData> result)
{
	vtkSmartPointer<vtkIterativeClosestPointTransform> icp = vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
	icp->SetSource(source);
	icp->SetTarget(target);
	icp->GetLandmarkTransform()->SetModeToAffine();
	icp->SetMaximumNumberOfIterations(10);
	//icp->StartByMatchingCentroidsOn();
	icp->Modified();
	icp->Update();
	vtkSmartPointer<vtkMatrix4x4> m = icp->GetMatrix();
	std::cout << "The resulting matrix after ICP is: " << *m << std::endl;

	vtkSmartPointer<vtkTransformPolyDataFilter> icpFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	icpFilter->SetInputData(source);
	icpFilter->SetTransform(icp);
	icpFilter->Update();
	result = icpFilter->GetOutput();
}


int main(int argc, char *argv[])
{
	if (argc != 6)
	{
		std::cerr << "Required parameters: \n\tsource_vtk \n\ttarget_vtk \n\ttarget_2_source_matrix \n\tsource_in_target_vtk_output \n\ttarget_with_source_scalars_vtk" << std::endl;
		return EXIT_FAILURE;
	}

	double element;
	std::string fn1 = argv[1];
	std::string fn2 = argv[2];
	std::string fn3 = argv[3];    
	std::string fn4 = argv[4];
	std::string fn5 = argv[5];

	vtkSmartPointer<vtkFileOutputWindow> fileOutputWindow = vtkSmartPointer<vtkFileOutputWindow>::New();
	fileOutputWindow->SetFileName("vtk_errors.txt");

	// Note that the SetInstance function is a static member of vtkOutputWindow.
	vtkOutputWindow::SetInstance(fileOutputWindow);
	
	
	vtkSmartPointer<vtkPolyDataReader> reader1 = vtkSmartPointer<vtkPolyDataReader>::New();
	reader1->SetFileName(fn1.c_str());
	reader1->Update();
	
	vtkSmartPointer<vtkPolyDataReader> reader2 = vtkSmartPointer<vtkPolyDataReader>::New();
	reader2->SetFileName(fn2.c_str());
	reader2->Update();


	vtkSmartPointer<vtkPolyData> source_poly = vtkSmartPointer<vtkPolyData>::New(); 
	source_poly = reader1->GetOutput();

	vtkSmartPointer<vtkPolyData> target_poly = vtkSmartPointer<vtkPolyData>::New();
	target_poly = reader2->GetOutput();

	
	vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
	ReadMatrix4x4(fn3.c_str(), matrix); 
	//matrix->Invert(); 
	
	//ofstream f(fn2.c_str()); 
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			element = matrix->GetElement(i, j); 
			cout << element << "\n";
		}	
	}

	
	vtkSmartPointer<vtkTransform> lm_transform = vtkSmartPointer<vtkTransform>::New();
	lm_transform->SetMatrix(matrix);

	
	vtkSmartPointer<vtkTransformPolyDataFilter> lmtFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	lmtFilter->SetInputData(source_poly);
	lmtFilter->SetTransform(lm_transform);
	lmtFilter->Update();
	

	// The source in target after landmark reg. matrix inversion 
	vtkSmartPointer<vtkPolyData> source_in_target_lm_reg = vtkSmartPointer<vtkPolyData>::New();
	source_in_target_lm_reg = lmtFilter->GetOutput();

	// The source in target after landmark inv + icp 
	vtkSmartPointer<vtkPolyData> source_in_target_lm_icp_reg = vtkSmartPointer<vtkPolyData>::New();

	cout << "Running ICP" << endl;
	//ICPRegistration(source_in_target_lm_reg, target_poly, source_in_target_lm_icp_reg);
	vtkSmartPointer<vtkIterativeClosestPointTransform> icp = vtkSmartPointer<vtkIterativeClosestPointTransform>::New();
	icp->SetSource(source_in_target_lm_reg);
	icp->SetTarget(target_poly);
	icp->GetLandmarkTransform()->SetModeToAffine();
	icp->SetMaximumNumberOfIterations(10);
	//icp->StartByMatchingCentroidsOn();
	icp->Modified();
	icp->Update();
	
	vtkSmartPointer<vtkTransformPolyDataFilter> icpFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
	icpFilter->SetInputData(source_in_target_lm_reg);
	icpFilter->SetTransform(icp);
	icpFilter->Update();
	source_in_target_lm_icp_reg = icpFilter->GetOutput();

	cout << "ICP completed " << endl;

	TransferPointDataToTargetPoly(source_in_target_lm_icp_reg, target_poly, source_poly->GetPointData()->GetScalars());
	
	vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
	writer->SetInputData(source_in_target_lm_icp_reg);
	writer->SetFileName(fn4.c_str());
	writer->Update();

	vtkSmartPointer<vtkPolyDataWriter> writer2 = vtkSmartPointer<vtkPolyDataWriter>::New();
	writer2->SetInputData(target_poly);
	writer2->SetFileName(fn5.c_str());
	writer2->Update();


	return EXIT_SUCCESS;
}