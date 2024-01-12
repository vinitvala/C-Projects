#include "hw7.h"

bst_sf* insert_bst_sf(matrix_sf *mat, bst_sf *root)
{

	bst_sf *node = malloc(sizeof(bst_sf));
	node->mat = mat;
	node->left_child = NULL;
	node->right_child = NULL;

	if(root == NULL)
		return node;

	bst_sf *curr = NULL;
	bst_sf *next = root;
	while(next != NULL)
	{
		curr = next;
		if(mat->name < curr->mat->name)
			next = next->left_child;
		else
			next = next->right_child;
	}

	if(mat->name < curr->mat->name)
		curr->left_child = node;
	else
		curr->right_child = node;

	return root;
}

matrix_sf* find_bst_sf(char name, bst_sf *root) {

	bst_sf *curr = root;
	while(curr != NULL)
	{
		if(name < curr->mat->name)
			curr = curr->left_child;
		else if(name == curr->mat->name)
			return curr->mat;
		else
			curr = curr->right_child;
	}

	return NULL;
}

void free_bst_sf(bst_sf *root) {

	if(root == NULL)
		return;

	free_bst_sf(root->left_child);
	free_bst_sf(root->right_child);
	free(root->mat);
	free(root);
}

matrix_sf* add_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
	matrix_sf *mat = malloc(sizeof(matrix_sf) + mat1->num_rows * mat1->num_cols * sizeof(int));
	
	mat->name = '_';
	mat->num_rows = mat1->num_rows;
	mat->num_cols = mat1->num_cols;

	for(int i = 0; i < mat->num_rows * mat->num_cols; i++)
		mat->values[i] = mat1->values[i] + mat2->values[i];

    return mat;
}

matrix_sf* mult_mats_sf(const matrix_sf *mat1, const matrix_sf *mat2) {
	matrix_sf *mat = malloc(sizeof(matrix_sf) + mat1->num_rows * mat2->num_cols * sizeof(int));
	
	mat->name = '_';
	mat->num_rows = mat1->num_rows;
	mat->num_cols = mat2->num_cols;

	for(int i = 0; i < mat->num_rows; i++)
		for(int j = 0; j < mat->num_cols; j++) {
			int sum = 0;
			for(int k = 0; k < mat1->num_cols; k++)
				sum += mat1->values[k + i * mat1->num_cols] * mat2->values[j + k * mat2->num_cols];

			mat->values[j + i * mat->num_cols] = sum;
		}

    return mat;
}

matrix_sf* transpose_mat_sf(const matrix_sf *mat) {
	matrix_sf *ret = malloc(sizeof(matrix_sf) + mat->num_rows * mat->num_cols * sizeof(int));
	
	ret->name = '_';
	ret->num_rows = mat->num_cols;
	ret->num_cols = mat->num_rows;

	for(int i = 0; i < ret->num_rows; i++)
		for(int j = 0; j < ret->num_cols; j++)
			ret->values[j + i * ret->num_cols] = mat->values[i + j * mat->num_cols];

    return ret;
}

matrix_sf* create_matrix_sf(char name, const char *expr) {
	int num_rows;
	int num_cols;

	sscanf(expr, "%d %d", &num_rows, &num_cols);

	matrix_sf *mat = malloc(sizeof(matrix_sf) + num_rows * num_cols * sizeof(int));
	mat->name = name;
	mat->num_rows = num_rows;
	mat->num_cols = num_cols;

	expr = strchr(expr, '[');
	expr++;

	for(int i = 0; i < num_rows * num_cols; i++)
	{
		while(expr[0] != '-' && !isdigit(expr[0]))
			expr++;

		sscanf(expr, "%d", &mat->values[i]);

		while(expr[0] == '-' || isdigit(expr[0]))
			expr++;
	}

	return mat;
}

char* infix2postfix_sf(char *infix) {

	int ssize = 0;
	int psize = 0;

	char *s = malloc(strlen(infix));
	char *p = malloc(strlen(infix)+1);

	for(int i = 0; i < strlen(infix); i++)
	{
		char c = infix[i];

		if(isspace(c))
			continue;

		if(isalpha(c) || c == '\'')
			p[psize++] = c;
		else if(c == ')')
		{
			while(s[--ssize] != '(')
				p[psize++] = s[ssize];
		}
		else if(c == '(' || ssize == 0 || s[ssize - 1] == '(' || s[ssize - 1] > c)
			s[ssize++] = c;
		else
		{
			while(ssize > 0 && s[ssize - 1] <= c && s[ssize - 1] != '(')
				p[psize++] = s[--ssize];

			s[ssize++] = c;
		}
	}

	while(ssize > 0)
		p[psize++] = s[--ssize];

	p[psize] = '\0';

	free(s);
	return p;
}

matrix_sf* evaluate_expr_sf(char name, char *expr, bst_sf *root) {
	int ssize = 0;
	matrix_sf **s = malloc(sizeof(matrix_sf *) * strlen(expr));

	char *p = infix2postfix_sf(expr);
	for(int i = 0; i < strlen(p); i++) {
		char c = p[i];

		if(isalpha(c))
			s[ssize++] = find_bst_sf(c, root);
		else
		{
			matrix_sf *mat2 = s[--ssize];
			matrix_sf *mat1 = NULL;
			
			if(c != '\'')
				mat1 = s[--ssize];

			switch(c)
			{
				case '\'':
					s[ssize++] = transpose_mat_sf(mat2);
					break;
				case '*':
					s[ssize++] = mult_mats_sf(mat1, mat2);
					break;
				case '+':
					s[ssize++] = add_mats_sf(mat1, mat2);
					break;
			}

			if(mat1 != NULL && !isalpha(mat1->name))
				free(mat1);

			if(!isalpha(mat2->name))
				free(mat2);
		}
	}

	free(p);

	matrix_sf *ret = s[0];
	ret->name = name;

	free(s);

	return ret;
}

matrix_sf *execute_script_sf(char *filename) {
	char *str = NULL;
	FILE *file = fopen(filename, "r");
	size_t max_line_size = MAX_LINE_LEN;

	bst_sf *root = NULL;
	matrix_sf *current_mat = NULL;

	while(getline(&str, &max_line_size, file) > 0)
	{
		if(current_mat != NULL)
			root = insert_bst_sf(current_mat, root);

		char c = str[0];
		char *expr = strchr(str, '=') + 1;

		if(strchr(str, '[') != NULL)
			current_mat = create_matrix_sf(c, expr);
		else
			current_mat = evaluate_expr_sf(c, expr, root); 
	}

	free(str);
	free_bst_sf(root);

	fclose(file);
	return current_mat;
}

// This is a utility function used during testing. Feel free to adapt the code to implement some of
// the assignment. Feel equally free to ignore it.
matrix_sf *copy_matrix(unsigned int num_rows, unsigned int num_cols, int values[]) {
    matrix_sf *m = malloc(sizeof(matrix_sf)+num_rows*num_cols*sizeof(int));
    m->name = '?';
    m->num_rows = num_rows;
    m->num_cols = num_cols;
    memcpy(m->values, values, num_rows*num_cols*sizeof(int));
    return m;
}

// Don't touch this function. It's used by the testing framework.
// It's been left here in case it helps you debug and test your code.
void print_matrix_sf(matrix_sf *mat) {
    assert(mat != NULL);
    assert(mat->num_rows <= 1000);
    assert(mat->num_cols <= 1000);
    printf("%d %d ", mat->num_rows, mat->num_cols);
    for (unsigned int i = 0; i < mat->num_rows*mat->num_cols; i++) {
        printf("%d", mat->values[i]);
        if (i < mat->num_rows*mat->num_cols-1)
            printf(" ");
    }
    printf("\n");
}
