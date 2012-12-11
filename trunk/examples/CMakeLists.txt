option(BUILD_EXAMPLES_OPENCV "Build examples using OpenCV" ON)
IF(BUILD_OPENGL_RENDERER)

	option(BUILD_EXAMPLES_GLUT "Build examples based on GLUT" ON)
	IF (BUILD_EXAMPLES_GLUT)
		
		IF(BUILD_INPUT_VMDirectShow)
			SUBDIRS(
			VMwithDirectShow
			VMwithDirectShowMultiVideo  
			VMwithDSDeviceController
			)
			IF (BUILD_INPUT_DS_PointGrey_CONTROLLER)
				SUBDIRS(VMwithDSPointGreyController)
			ENDIF (BUILD_INPUT_DS_PointGrey_CONTROLLER)
		ENDIF(BUILD_INPUT_VMDirectShow)

		SUBDIRS(
		multiCamera
		videoFileControl 
		imageSequence
		)

		IF(BUILD_EXAMPLES_OPENCV)
			SUBDIRS(
			drawing2D
			opencvSimple 
			opencvLKdemo 
			)
		ENDIF(BUILD_EXAMPLES_OPENCV)

	ENDIF (BUILD_EXAMPLES_GLUT)

	option(BUILD_EXAMPLES_QT "Build examples based on QT" OFF)
	IF (BUILD_EXAMPLES_QT)
		SUBDIRS( 
		VMwithQT
		multiInstancesQT
		)
	ENDIF (BUILD_EXAMPLES_QT)

	option(BUILD_EXAMPLES_CUDA "Build examples using CUDA" OFF)

	IF(BUILD_EXAMPLES_CUDA)
		SUBDIRS( 
		cuda
		)
	ENDIF(BUILD_EXAMPLES_CUDA)

	IF(BUILD_INPUT_VMPointGrey2)
		SUBDIRS( 
		VMWithPointGreySimple
		VMWithPointGreyAdvanced
		)
	ENDIF(BUILD_INPUT_VMPointGrey2)
	
	IF(BUILD_INPUT_VMIDSuEye)
		SUBDIRS( 
		IDSuEye
		)
	ENDIF(BUILD_INPUT_VMIDSuEye)

ENDIF(BUILD_OPENGL_RENDERER)

IF(BUILD_EXAMPLES_OPENCV)
	SUBDIRS( 
	opencvHighgui
	)
ENDIF(BUILD_EXAMPLES_OPENCV)

