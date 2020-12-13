#include "GraphicModel.h"
#include "Core.h"

namespace gl3d 
{

	void GraphicModel::loadFromData(size_t vertexSize,
			float *vercies, size_t indexSize, unsigned int *indexes)
	{

		gl3dAssertComment(vertexSize % 3 == 0, "Index count must be multiple of 3");

		if(indexSize && indexes)
		{
		
		}else
		{
			glGenVertexArrays(1, &vertexArray);
			glBindVertexArray(vertexArray);

			glGenBuffers(1, &vertexBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
			glBufferData(GL_ARRAY_BUFFER, vertexSize, vercies, GL_STATIC_DRAW);
			
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

			primitiveCount = vertexSize / sizeof(float);


			glBindVertexArray(0);

		}

		

	}

	void GraphicModel::clear()
	{
		glDeleteBuffers(1, &vertexBuffer);
		glDeleteBuffers(1, &indexBuffer);

		glDeleteVertexArrays(1, &vertexArray);

		vertexBuffer = 0;
		indexBuffer = 0;
		primitiveCount = 0;
		vertexArray = 0;
	}

	void GraphicModel::draw()
	{
		glBindVertexArray(vertexArray);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);


		if (indexBuffer)
		{

		}
		else
		{
			glDrawArrays(GL_TRIANGLES, 0, primitiveCount);
		}


		glBindVertexArray(0);
	}



};
