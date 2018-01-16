﻿#include "StreamStore.h"

// Creation / Destruction

StreamObjectBase::StreamObjectBase(const FBModel* ModelPointer, const TSharedPtr<ILiveLinkProvider> StreamProvider, std::initializer_list<FString> Options)
	: RootModel(ModelPointer), Provider(StreamProvider), ConnectionOptions(Options), bIsActive(true), StreamingMode(0)
{
	FString ModelLongName(ANSI_TO_TCHAR(RootModel->LongName));
	FString RightString;
	ModelLongName.Split(TEXT(":"), &ModelLongName, &RightString);
	SubjectName = FName(*ModelLongName);
};

StreamObjectBase::~StreamObjectBase()
{
	Provider->ClearSubject(SubjectName);
};

// Stream Object Interface

FString StreamObjectBase::GetStreamOptions()
{
	return FString::Join(ConnectionOptions, _T("~"));
};

FName StreamObjectBase::GetSubjectName() const 
{ 
	return SubjectName; 
};

void StreamObjectBase::UpdateSubjectName(FName NewSubjectName)
{
	Provider->ClearSubject(SubjectName);
	SubjectName = NewSubjectName;
	UpdateFromModel();
};


int StreamObjectBase::GetStreamingMode() const
{
	return StreamingMode;
};

void StreamObjectBase::UpdateStreamingMode(int NewStreamingMode)
{
	StreamingMode = NewStreamingMode;
	UpdateFromModel();
};


bool StreamObjectBase::GetActiveStatus() const
{
	return bIsActive;
};

void StreamObjectBase::UpdateActiveStatus(bool bIsNowActive)
{
	bIsActive = bIsNowActive;
	UpdateFromModel();
};


// Equality comparison

bool StreamObjectBase::operator==(const StreamObjectBase &other) const {
	return (this->RootModel == other.RootModel);
};


// Utility Functions

FTransform StreamObjectBase::MobuTransformToUnreal(FBMatrix& MobuTransfrom)
{
	FBMatrix MobuTransformUnrealSpace;
	FBTVector TVector;
	FBSVector SVector;
	FBQuaternion Quat;
	for (int j = 0; j < 4; ++j)
	{
		if (j == 1)
		{
			MobuTransformUnrealSpace(j, 0) = -MobuTransfrom(j, 0);
			MobuTransformUnrealSpace(j, 1) = MobuTransfrom(j, 1);
			MobuTransformUnrealSpace(j, 2) = -MobuTransfrom(j, 2);
			MobuTransformUnrealSpace(j, 3) = -MobuTransfrom(j, 3);
		}
		else
		{
			MobuTransformUnrealSpace(j, 0) = MobuTransfrom(j, 0);
			MobuTransformUnrealSpace(j, 1) = -MobuTransfrom(j, 1);
			MobuTransformUnrealSpace(j, 2) = MobuTransfrom(j, 2);
			MobuTransformUnrealSpace(j, 3) = MobuTransfrom(j, 3);
		}
	}

	FBMatrixToTranslation(TVector, MobuTransformUnrealSpace);
	FBMatrixToQuaternion(Quat, MobuTransformUnrealSpace);
	FBMatrixToScaling(SVector, MobuTransformUnrealSpace);

	FTransform UnrealTransform;
	UnrealTransform.SetRotation(FQuat(Quat[0], Quat[1], Quat[2], Quat[3]));
	UnrealTransform.SetTranslation(FVector(TVector[0], TVector[1], TVector[2]));
	UnrealTransform.SetScale3D(FVector(SVector[0], SVector[1], SVector[2]));

	return UnrealTransform;
};

FTransform StreamObjectBase::UnrealTransformFromModel(FBModel* MobuModel, bool bIsGlobal)
{
	FBMatrix MobuTransform;
	FBMatrix MatOffset;

	MobuModel->GetMatrix(MobuTransform, kModelTransformation, bIsGlobal, nullptr);

	// Y-Up Correction
	FBRVector RotOffset(90, 0, 0);
	FBRotationToMatrix(MatOffset, RotOffset);
	FBMatrixMult(MobuTransform, MatOffset, MobuTransform);

	return MobuTransformToUnreal(MobuTransform);
};



